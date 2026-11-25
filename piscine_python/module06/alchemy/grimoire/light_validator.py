"""Module 6, Part IV: Light spell validator."""


def validate_ingredients(ingredients: str) -> str:
    """Return VALID when every token is an allowed light ingredient."""
    allowed = {"earth", "wind", "fire", "water", "and"}
    tokens = [token.lower() for token in ingredients.replace(",", " ").split()]
    if tokens and all(token in allowed for token in tokens):
        return "VALID"
    return "INVALID"
