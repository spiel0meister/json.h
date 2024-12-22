#define ALI_REMOVE_PREFIX
#define ALI_IMPLEMENTATION
#include "ali.h"

#define JSON_IMPLEMENTATION
#include "json.h"

int main(void) {
    AliSb sb = {0};
    if (!sb_read_file(&sb, "products.json")) return 1;
    JsonLexer lexer = json_lexer(sb.data, sb.count);

    JsonValue value;
    if (!json_lexer_parse_value(&lexer, &value)) return 1;

    JsonObject* value_as_object = json_value_as_object(&value);
    if (value_as_object == NULL) return 1;

    JsonValue* products_json = json_object_find_value(value_as_object, "products");
    if (products_json == NULL) return 1;

    JsonArray* products = json_value_as_array(products_json);
    if (products == NULL) return 1;

    for (size_t i = 0; i < products->len; ++i) {
        JsonValue* value = json_array_get_item(products, i);
        json_stringify(value);
        printf("\n");
    }

    return 0;
}
