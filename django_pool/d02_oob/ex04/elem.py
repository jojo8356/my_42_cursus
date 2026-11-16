# Exercice 04 : Une classe de base ft. RMS
# Dossier : ex04/ - Fichier : elem.py


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


def main():
    elem = Elem(
        "div",
        attributes="class=\"test\"",
        content="Hello",
        tag_type="double"
    )
    print(elem)

    elem.add_content(" world")
    print(elem)

    img = Elem("img", attributes="src=\"image.png\"", content="", tag_type="simple")
    print(img)


if __name__ == '__main__':
    main()
