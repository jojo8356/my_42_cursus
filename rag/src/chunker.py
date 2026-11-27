"""
Document chunking for the RAG project.

Three dedicated chunkers, one per supported file type, plus file-path
enrichment and exact source locations.

- Python: parsed with the standard ``ast`` module. Each top-level class and
  function becomes a chunk. Large definitions are split recursively with a
  configurable overlap. Invalid syntax falls back to plain-text chunking.
- Markdown: the heading hierarchy is preserved; each chunk carries the
  active heading path.
- Plain text: split at natural boundaries (blank lines, paragraphs,
  punctuation) rather than fixed-width positions.

Every chunk stores exact ``first_character_index`` / ``last_character_index``
offsets into the original file.
"""
import ast
import re

from models import Chunk, stable_chunk_id

# Default chunking parameters.
DEFAULT_MAX_CHUNK_SIZE = 2000
DEFAULT_OVERLAP = 200

PYTHON_EXTENSIONS = {".py"}
MARKDOWN_EXTENSIONS = {".md", ".markdown"}
TEXT_EXTENSIONS = {".txt", ".rst", ".text"}
# Files without a recognizable extension handled as plain text.
TEXT_FILENAMES = {"dockerfile", "makefile", "readme", "license", "changelog"}


# --------------------------------------------------------------------- helpers
def _line_offsets(text):
    """Return the cumulative character offset of the start of each line."""
    offsets = [0]
    for line in text.splitlines(keepends=True):
        offsets.append(offsets[-1] + len(line))
    return offsets


def _pos_to_offset(line_offsets, lineno, col_offset):
    """Convert a 1-based (line, column) AST position to a character offset."""
    if 1 <= lineno <= len(line_offsets):
        return line_offsets[lineno - 1] + col_offset
    return 0


def _hard_split(text, max_size, overlap):
    """Split an over-long string into windows of <= max_size with overlap."""
    if len(text) <= max_size:
        return [text]
    chunks = []
    start = 0
    step = max(1, max_size - overlap)
    while start < len(text):
        end = min(start + max_size, len(text))
        chunks.append(text[start:end])
        if end >= len(text):
            break
        start += step
    return chunks


def _split_text_by_natural_boundaries(text, max_size, overlap):
    """Split text at natural boundaries into <= max_size chunks with overlap."""
    if not text:
        return []
    # Split into paragraphs (blank-line separated), keeping separators.
    parts = re.split(r"(\n\s*\n)", text)
    # Reassemble paragraphs, but break over-long paragraphs by lines.
    pieces = []
    for part in parts:
        if part == "":
            continue
        if len(part) <= max_size:
            pieces.append(part)
        else:
            # Break long paragraph by lines into max_size windows.
            lines = part.splitlines(keepends=True)
            buf = ""
            for line in lines:
                if len(buf) + len(line) > max_size and buf:
                    pieces.append(buf)
                    # Keep overlap: retain the tail of the previous chunk.
                    keep = buf[-overlap:] if overlap > 0 else ""
                    buf = keep + line
                else:
                    buf += line
            if buf:
                # If a single line/window still exceeds max_size, hard-split.
                if len(buf) > max_size:
                    pieces.extend(_hard_split(buf, max_size, overlap))
                else:
                    pieces.append(buf)

    # Merge tiny trailing pieces into the previous chunk when possible, and
    # guarantee every resulting chunk respects max_size.
    chunks = []
    current = ""
    for piece in pieces:
        if current and len(current) + len(piece) <= max_size:
            current += piece
        else:
            if current:
                chunks.append(current)
            # A single piece may still exceed max_size (rare); hard-split it.
            if len(piece) > max_size:
                chunks.extend(_hard_split(piece, max_size, overlap))
                current = ""
            else:
                current = piece
    if current:
        chunks.append(current)
    return chunks


def _extract_path_keywords(file_path):
    """Extract searchable keywords from a file path."""
    keywords = []
    for component in file_path.replace("\\", "/").split("/"):
        if not component:
            continue
        # Split on non-alphanumeric to get terms (e.g. tool_calling -> tool calling).
        for term in re.split(r"[^A-Za-z0-9]+", component):
            if term and term not in keywords:
                keywords.append(term.lower())
    return keywords


# --------------------------------------------------------------------- python
def _extract_symbols(node):
    """Collect top-level class/function/method names from an AST."""
    symbols = []
    for child in ast.walk(node):
        if isinstance(child, (ast.ClassDef, ast.FunctionDef, ast.AsyncFunctionDef)):
            if isinstance(child, ast.ClassDef):
                symbols.append(child.name)
                # methods
                for sub in child.body:
                    if isinstance(sub, (ast.FunctionDef, ast.AsyncFunctionDef)):
                        symbols.append(child.name + "." + sub.name)
            elif child.col_offset == 0:
                symbols.append(child.name)
    return symbols


def _python_definitions(source, line_offsets):
    """Return a list of (start_offset, end_offset, name, is_class) for
    top-level definitions, sorted by position."""
    try:
        tree = ast.parse(source)
    except (SyntaxError, ValueError):
        return None  # signal fallback to plain text

    definitions = []
    for node in tree.body:
        if isinstance(node, (ast.FunctionDef, ast.AsyncFunctionDef, ast.ClassDef)):
            start = _pos_to_offset(line_offsets, node.lineno, node.col_offset)
            end = _pos_to_offset(
                line_offsets, getattr(node, "end_lineno", node.lineno),
                getattr(node, "end_col_offset", node.col_offset))
            name = node.name
            definitions.append((start, end, name, isinstance(node, ast.ClassDef)))
    definitions.sort(key=lambda d: d[0])
    return definitions


def chunk_python(file_path, source, max_size=DEFAULT_MAX_CHUNK_SIZE,
                 overlap=DEFAULT_OVERLAP):
    """Chunk a Python source file using AST-aware splitting."""
    line_offsets = _line_offsets(source)
    definitions = _python_definitions(source, line_offsets)
    path_keywords = _extract_path_keywords(file_path)

    if definitions is None:
        # Invalid syntax -> fall back to plain text chunking.
        return chunk_text(file_path, source, max_size, overlap,
                          file_type="python", path_keywords=path_keywords)

    all_symbols = []
    chunks = []
    cursor = 0  # start of the "imports / preamble" region

    # Preamble: everything before the first definition.
    first_start = definitions[0][0]
    if first_start > 0:
        preamble = source[cursor:first_start]
        if preamble.strip():
            chunks.append(_make_chunk(
                file_path, preamble, cursor, first_start, "python",
                [], [], path_keywords))

    for start, end, name, is_class in definitions:
        all_symbols.append(name)
        body = source[start:end]
        # Split large definitions with overlap.
        if len(body) > max_size:
            sub_chunks = _split_text_by_natural_boundaries(body, max_size, overlap)
            offset = start
            for i, sub in enumerate(sub_chunks):
                sub_start = offset
                sub_end = offset + len(sub)
                chunks.append(_make_chunk(
                    file_path, sub, sub_start, sub_end, "python",
                    [name], [], path_keywords))
                offset = sub_end
        else:
            chunks.append(_make_chunk(
                file_path, body, start, end, "python", [name], [],
                path_keywords))

    # Attach the symbol list to the chunks for the AST symbol bonus.
    for ch in chunks:
        ch.symbols = sorted(set(all_symbols))
    return chunks


# --------------------------------------------------------------------- markdown
_HEADING_RE = re.compile(r"^(#{1,6})\s+(.*)$")


def chunk_markdown(file_path, source, max_size=DEFAULT_MAX_CHUNK_SIZE,
                   overlap=DEFAULT_OVERLAP):
    """Chunk a Markdown file, preserving the heading hierarchy."""
    line_offsets = _line_offsets(source)
    path_keywords = _extract_path_keywords(file_path)

    lines = source.splitlines(keepends=True)
    # Build sections: each section = (heading_path, start_line, end_line).
    sections = []
    heading_stack = []  # list of (level, title)
    current_start = 0
    current_heading = []

    def flush(end_line):
        nonlocal current_start, current_heading
        if end_line > current_start:
            sections.append((list(current_heading), current_start, end_line))
        current_start = end_line

    for idx, line in enumerate(lines):
        m = _HEADING_RE.match(line)
        if m:
            flush(idx)
            level = len(m.group(1))
            title = m.group(2).strip()
            # Trim the stack to the parent level.
            while heading_stack and heading_stack[-1][0] >= level:
                heading_stack.pop()
            heading_stack.append((level, title))
            current_heading = [t for _, t in heading_stack]

    flush(len(lines))

    chunks = []
    for heading_path, start_line, end_line in sections:
        start_offset = line_offsets[start_line]
        end_offset = line_offsets[end_line]
        text = source[start_offset:end_offset]
        if not text.strip():
            continue
        if len(text) > max_size:
            sub_chunks = _split_text_by_natural_boundaries(text, max_size, overlap)
            offset = start_offset
            for sub in sub_chunks:
                sub_start = offset
                sub_end = offset + len(sub)
                chunks.append(_make_chunk(
                    file_path, sub, sub_start, sub_end, "markdown",
                    [], heading_path, path_keywords))
                offset = sub_end
        else:
            chunks.append(_make_chunk(
                file_path, text, start_offset, end_offset, "markdown",
                [], heading_path, path_keywords))
    return chunks


# --------------------------------------------------------------------- text
def chunk_text(file_path, source, max_size=DEFAULT_MAX_CHUNK_SIZE,
               overlap=DEFAULT_OVERLAP, file_type="text", path_keywords=None):
    """Chunk a plain-text document at natural boundaries."""
    if path_keywords is None:
        path_keywords = _extract_path_keywords(file_path)
    pieces = _split_text_by_natural_boundaries(source, max_size, overlap)
    chunks = []
    cursor = 0
    for piece in pieces:
        idx = source.find(piece, cursor)
        if idx == -1:
            idx = cursor
        start = idx
        end = idx + len(piece)
        chunks.append(_make_chunk(
            file_path, piece, start, end, file_type, [], [], path_keywords))
        cursor = end
    return chunks


def _make_chunk(file_path, text, start, end, file_type, symbols, heading_path,
                path_keywords):
    chunk = Chunk(
        file_path=file_path,
        text=text,
        first_character_index=start,
        last_character_index=end,
        file_type=file_type,
        symbols=list(symbols),
        heading_path=list(heading_path),
        path_keywords=list(path_keywords),
    )
    chunk.chunk_id = stable_chunk_id(
        file_path, start, end, text)
    return chunk


# --------------------------------------------------------------------- dispatcher
def detect_file_type(file_path):
    """Return 'python', 'markdown' or 'text' for a file path."""
    lower = file_path.lower()
    ext = "." + lower.rsplit(".", 1)[-1] if "." in lower else ""
    if ext in PYTHON_EXTENSIONS:
        return "python"
    if ext in MARKDOWN_EXTENSIONS:
        return "markdown"
    name = lower.rsplit("/", 1)[-1]
    if name in TEXT_FILENAMES or ext in TEXT_EXTENSIONS:
        return "text"
    return "text"


def chunk_file(file_path, source, max_size=DEFAULT_MAX_CHUNK_SIZE,
               overlap=DEFAULT_OVERLAP):
    """Dispatch to the appropriate chunker based on the file type."""
    file_type = detect_file_type(file_path)
    if file_type == "python":
        return chunk_python(file_path, source, max_size, overlap)
    if file_type == "markdown":
        return chunk_markdown(file_path, source, max_size, overlap)
    return chunk_text(file_path, source, max_size, overlap)
