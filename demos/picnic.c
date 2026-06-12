#define XAO_IMPL
#include "xao.h"

#include <stdio.h>
#include <stdlib.h>

#define SF(v) ((int)((v).end - (v).start)), (v.start)

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
    printf("A %.*s-weight %.*s\n", SF(ripeness), SF(name));
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
