# Exercice 05 : Faites vos propres éléments!
# Dossier : ex05/ - Fichiers : elem.py, elements.py


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


def main():
    page = Html([Head([Title("Test")]), Body([P("Hello world")])])
    print(page)

    # Reproduction de la structure demandée
    html = Html([
        Body([
            Div([
                H1("Hello ground!"),
                Br(),
                P("Oh no, not again!"),
                Br(),
                Img()
            ])
        ])
    ])
    print(html)


if __name__ == '__main__':
    main()
