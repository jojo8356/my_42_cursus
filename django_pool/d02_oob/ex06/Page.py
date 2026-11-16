# Exercice 06 : Validation
# Dossier : ex06/ - Fichiers : Page.py, elem.py, elements.py


class Elem:
    def __init__(self, tag_name, attributes=None, content=None, tag_type="double"):
        self.tag_name = tag_name
        self.attributes = attributes if attributes else ""
        self.content = content if content else ""
        self.tag_type = tag_type

    def __str__(self):
        attr_str = ""
        if self.attributes:
            attr_str = f" {self.attributes}"
        if self.tag_type == "double":
            return f"<{self.tag_name}{attr_str}>{self.content}</{self.tag_name}>"
        else:
            return f"<{self.tag_name}{attr_str}/>"

    def add_content(self, content):
        if self.content:
            self.content += content
        else:
            self.content = content

    class InvalidElementException(Exception):
        pass


class Text:
    def __init__(self, content):
        self.content = content

    def __str__(self):
        return str(self.content)


class Html(Elem):
    def __init__(self, content=None):
        super().__init__("html", content=content)


class Head(Elem):
    def __init__(self, content=None):
        super().__init__("head", content=content)


class Body(Elem):
    def __init__(self, content=None):
        super().__init__("body", content=content)


class Title(Elem):
    def __init__(self, content=None):
        super().__init__("title", content=content)


class Meta(Elem):
    def __init__(self, content=None):
        super().__init__("meta", tag_type="simple")


class Img(Elem):
    def __init__(self, content=None):
        super().__init__("img", tag_type="simple")


class Table(Elem):
    def __init__(self, content=None):
        super().__init__("table", content=content)


class Th(Elem):
    def __init__(self, content=None):
        super().__init__("th", content=content)


class Tr(Elem):
    def __init__(self, content=None):
        super().__init__("tr", content=content)


class Td(Elem):
    def __init__(self, content=None):
        super().__init__("td", content=content)


class Ul(Elem):
    def __init__(self, content=None):
        super().__init__("ul", content=content)


class Ol(Elem):
    def __init__(self, content=None):
        super().__init__("ol", content=content)


class Li(Elem):
    def __init__(self, content=None):
        super().__init__("li", content=content)


class H1(Elem):
    def __init__(self, content=None):
        super().__init__("h1", content=content)


class H2(Elem):
    def __init__(self, content=None):
        super().__init__("h2", content=content)


class P(Elem):
    def __init__(self, content=None):
        super().__init__("p", content=content)


class Div(Elem):
    def __init__(self, content=None):
        super().__init__("div", content=content)


class Span(Elem):
    def __init__(self, content=None):
        super().__init__("span", content=content)


class Hr(Elem):
    def __init__(self, content=None):
        super().__init__("hr", tag_type="simple")


class Br(Elem):
    def __init__(self, content=None):
        super().__init__("br", tag_type="simple")


VALID_TAGS = {"html", "head", "body", "title", "meta", "img", "table",
              "th", "tr", "td", "ul", "ol", "li", "h1", "h2", "p",
              "div", "span", "hr", "br", "Text"}


class Page:
    def __init__(self, content):
        self.content = content

    def is_valid(self):
        return self._validate(self.content)

    def _validate(self, node):
        if isinstance(node, Text):
            return True

        if not isinstance(node, Elem):
            return False

        if node.tag_name not in VALID_TAGS and not isinstance(node, Text):
            return False

        if node.tag_name == "html":
            children = self._get_children(node.content)
            if len(children) != 2:
                return False
            if not isinstance(children[0], Head):
                return False
            if not isinstance(children[1], Body):
                return False
            if not self._validate(children[0]):
                return False
            if not self._validate(children[1]):
                return False

        elif node.tag_name == "head":
            children = self._get_children(node.content)
            if len(children) != 1 or not isinstance(children[0], Title):
                return False
            if not self._validate(children[0]):
                return False

        elif node.tag_name in ("body", "div"):
            children = self._get_children(node.content)
            for child in children:
                if not isinstance(child, (H1, H2, Div, Table, Ul, Ol, Span, Text)):
                    return False
                if not self._validate(child):
                    return False

        elif node.tag_name in ("title", "h1", "h2", "li", "th", "td"):
            children = self._get_children(node.content)
            if len(children) != 1 or not isinstance(children[0], Text):
                return False

        elif node.tag_name == "p":
            children = self._get_children(node.content)
            for child in children:
                if not isinstance(child, Text):
                    return False

        elif node.tag_name == "span":
            children = self._get_children(node.content)
            for child in children:
                if not isinstance(child, (Text, P)):
                    return False
                if not self._validate(child):
                    return False

        elif node.tag_name in ("ul", "ol"):
            children = self._get_children(node.content)
            if len(children) < 1:
                return False
            for child in children:
                if not isinstance(child, Li):
                    return False
                if not self._validate(child):
                    return False

        elif node.tag_name == "tr":
            children = self._get_children(node.content)
            if len(children) < 1:
                return False
            for child in children:
                if not isinstance(child, (Th, Td)):
                    return False
                if not self._validate(child):
                    return False

        elif node.tag_name == "table":
            children = self._get_children(node.content)
            for child in children:
                if not isinstance(child, Tr):
                    return False
                if not self._validate(child):
                    return False

        return True

    def _get_children(self, content):
        if isinstance(content, list):
            return content
        return [content]

    def __str__(self):
        html = str(self.content)
        if self.content.tag_name == "html":
            return "<!DOCTYPE html>\n" + html
        return html

    def write_to_file(self, filename):
        with open(filename, 'w') as f:
            f.write(str(self))


def main():
    # Page valide
    page = Page(Html([
        Head([Title("Test")]),
        Body([
            H1("Hello"),
            P("Test paragraph")
        ])
    ]))
    print("Valid:", page.is_valid())
    print(page)

    # Page invalide
    page2 = Page(Html([
        Head([Title("Test")]),
        Body([
            Ul([P("Mauvaise liste")])
        ])
    ]))
    print("Valid:", page2.is_valid())


if __name__ == '__main__':
    main()
