"""Module 6, Part IV: Dark spell validator."""


def dark_validate(ingredients: str) -> str:
    """Return VALID when every token is an allowed dark ingredient."""
    allowed = {"shadow", "night", "darkness", "and"}
    tokens = [token.lower() for token in ingredients.replace(",", " ").split()]
    if tokens and all(token in allowed for token in tokens):
        return "VALID"
    return "INVALID"
