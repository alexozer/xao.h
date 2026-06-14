# xao.h

A tiny XML parsing library in ~200 lines of C.

Inspired by [sj](https://github.com/rxi/sj.h).

## Features

- Zero memory allocation
- Fairly ergonomic API for traversing elements, attributes, and content
- Common escape codes (`&lt;`, `&gt;`, `&amp;`, `&apos;`, and `&quot;`)
    - These are unescaped in-place to avoid additional allocation
- Fast and O(n) performance
- Basic parse error messages

## Limitations

Does not support:

- Strict invalid XML detection - garbage XML can just parse garbage results
- Document Type Definition (DTD) parsing nor validation
- Numeric character escape codes
- UTF-8 validation
- Probably plenty of other cursed nonsense I'm ignorant of

## Demo

`picnic.xml`:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<PicnicBasket>
    <Fruit weight="2" ripeness="medium">Apple</Fruit>
    <Fruit weight="3" ripeness="high">Watermelon</Fruit>
    <Sandwich>
        <Shell>Bagel</Shell>
        <Topping>Egg</Topping>
        <Topping>Tomato</Topping>
        <Topping>Bacon</Topping>
    </Sandwich>
    <Journal>There&apos;s nothing like writing by the lake on a beautiful day.</Journal>
</PicnicBasket>
```

`main.c`:

```c
#define XAO_IMPL
#include "xao.h"

#include <stdio.h>
#include <stdlib.h>

#define SF(v) ((int)((v).end - (v).start)), ((v).start)

bool eq(xao_Value v, const char *s) {
    size_t size = v.end - v.start;
    return size == strlen(s) && memcmp(v.start, s, size) == 0;
}

char *read_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* xml = calloc(size + 1, 1);
    fread(xml, 1, size, fp);
    fclose(fp);
    return xml;
}

void parse_fruit(xao_Reader *r, xao_Value fruit) {
    xao_Value ripeness = {};
    xao_Value name = {};

    xao_Value key = {};
    xao_Value value = {};
    while (xao_iter_attrs(r, fruit, &key, &value)) {
        if (eq(key, "ripeness")) { ripeness = value; }
    }
    xao_iter_content(r, fruit, &name);
    printf("A %.*s-ripeness %.*s\n", SF(ripeness), SF(name));
}

void parse_picnic(char *xml) {
    xao_Reader r = xao_reader(xml, strlen(xml));
    xao_Value root = {};
    xao_Value basket = {};
    while (xao_iter_tags(&r, root, &basket)) {
        if (eq(basket, "PicnicBasket")) {
            xao_Value obj = {};
            while (xao_iter_tags(&r, basket, &obj)) {
                if (eq(obj, "Fruit")) {
                    parse_fruit(&r, obj);
                } else if (eq(obj, "Journal")) {
                    xao_Value journal_entry = {};
                    xao_iter_content(&r, obj, &journal_entry);
                    printf("Journal peek: \"%.*s\"\n", SF(journal_entry));
                }
            }
        }
    }

    if (r.error != NULL) {
        fprintf(stderr, "Parse error: %s\n", r.error);
    }
}

int main() {
    char *xml = read_file("picnic.xml");
    parse_picnic(xml);
    free(xml);
    return 0;
}
```

Outputs:

```
A medium-ripeness Apple
A high-ripeness Watermelon
Journal peek: "There's nothing like writing by the lake on a beautiful day."
```
