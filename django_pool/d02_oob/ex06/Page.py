# Exercice 06 : Validation
# Dossier : ex06/ - Fichiers : Page.py, elem.py, elements.py
from elem import Elem, Text
from elements import (Html, Head, Body, Title, Meta, Img, Table, Th, Tr, Td,
                      Ul, Ol, Li, H1, H2, P, Div, Span, Hr, Br)


class Page:
    def __init__(self, elem):
        if not isinstance(elem, Elem):
            raise Elem.ValidationError()
        self.elem = elem

    def __str__(self):
        result = ''
        if isinstance(self.elem, Html):
            result += '<!DOCTYPE html>\n'
        result += str(self.elem)
        return result

    def write_to_file(self, path):
        with open(path, 'w') as f:
            f.write(str(self))

    def is_valid(self):
        return self._check(self.elem)

    def _check(self, elem):
        allowed = (Html, Head, Body, Title, Meta, Img, Table, Th, Tr, Td,
                   Ul, Ol, Li, H1, H2, P, Div, Span, Hr, Br)
        if not isinstance(elem, allowed) and type(elem) is not Text:
            return False
        if type(elem) is Text or isinstance(elem, (Meta, Img, Hr, Br)):
            return True

        content = elem.content

        if isinstance(elem, Html):
            return (len(content) == 2 and isinstance(content[0], Head)
                    and isinstance(content[1], Body)
                    and all(self._check(child) for child in content))

        if isinstance(elem, Head):
            return (len(content) == 1 and isinstance(content[0], Title)
                    and self._check(content[0]))

        if isinstance(elem, (Body, Div)):
            return all(isinstance(child, (H1, H2, Div, Table, Ul, Ol, Span))
                       or type(child) is Text for child in content) \
                and all(self._check(child) for child in content)

        if isinstance(elem, (Title, H1, H2, Li, Th, Td)):
            return len(content) == 1 and type(content[0]) is Text

        if isinstance(elem, P):
            return all(type(child) is Text for child in content)

        if isinstance(elem, Span):
            return all(isinstance(child, (Text, P)) for child in content) \
                and all(self._check(child) for child in content)

        if isinstance(elem, (Ul, Ol)):
            return (len(content) >= 1
                    and all(isinstance(child, Li) for child in content)
                    and all(self._check(child) for child in content))

        if isinstance(elem, Tr):
            if len(content) < 1:
                return False
            if not all(isinstance(child, (Th, Td)) for child in content):
                return False
            return all(isinstance(child, type(content[0])) for child in content)

        if isinstance(elem, Table):
            return all(isinstance(child, Tr) for child in content) \
                and all(self._check(child) for child in content)

        return False


def main():
    page = Page(Html([
        Head(Title(Text('"Hello ground!"'))),
        Body([
            H1(Text('"Oh no, not again!"')),
            Img(attr={'src': 'http://i.imgur.com/pfp3T.jpg'}),
        ]),
    ]))
    print(page)
    print('is_valid:', page.is_valid())
    page.write_to_file('page.html')


if __name__ == '__main__':
    main()
