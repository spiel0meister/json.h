#ifndef JSON_H_
#define JSON_H_

#include <stddef.h>

typedef enum {
    JSON_NUMBER,
    JSON_BOOLEAN,
    JSON_STRING,
    JSON_ARRAY,
    JSON_OBJECT,
}JsonValueType;

typedef struct JsonObjectItem JsonObjectItem;
typedef struct JsonArrayNode JsonArrayNode;

typedef struct {
    size_t len;
    JsonArrayNode* next;
}JsonArray;

typedef struct {
    size_t len;
    JsonObjectItem* next;
}JsonObject;

typedef union {
    double number;
    bool boolean;
    char* string;
    JsonObject object;
    JsonArray array;
}JsonValueAs;

typedef struct {
    JsonValueType type;
    JsonValueAs as;
}JsonValue;

struct JsonObjectItem {
    char* key;
    JsonValue value;
    JsonObjectItem* next;
};

struct JsonArrayNode {
    JsonValue value;
    JsonArrayNode* next;
};

JsonValue json_value_string(char* string);
JsonValue json_value_number(double number);
JsonValue json_value_boolean(bool boolean);
JsonValue json_value_array(void);
JsonValue json_value_object(void);

JsonValue* json_array_get_item(JsonArray* array, size_t index);
JsonValue* json_object_get_item(JsonObject* object, size_t index);

JsonValue* json_object_find_value(JsonObject* object, char* key);

double* json_value_as_number(JsonValue* value);
char** json_value_as_string(JsonValue* value);
bool* json_value_as_boolean(JsonValue* value);
JsonObject* json_value_as_object(JsonValue* value);
JsonArray* json_value_as_array(JsonValue* value);

void json_stringify(JsonValue* value);

typedef struct {
    const char* content_start;
    size_t content_len;

    const char* cursor;
}JsonLexer;

JsonLexer json_lexer(const char* content_start, size_t content_size);
bool json_lexer_parse_value(JsonLexer* lexer, JsonValue* value);

#endif // JSON_H_

#ifdef JSON_IMPLEMENTATION

#ifndef JSON_ARENA_MAX_SIZE
#define JSON_ARENA_MAX_SIZE (1 << 20)
#endif // JSON_ARENA_MAX_SIZE
uint8_t json_arena[JSON_ARENA_MAX_SIZE] = {0};
size_t json_arena_size = 0;

size_t json_arena_stamp() {
    return json_arena_size;
}

void json_arena_rewind(size_t stamp) {
    json_arena_size = stamp;
}

void* json_alloc(size_t size) {
    void* ptr = &json_arena[json_arena_size];
    json_arena_size += size;
    return ptr;
}

char* json_strndup(const char* str, size_t len) {
    char* copy = json_alloc(len + 1);
    memcpy(copy, str, len);
    copy[len] = 0;
    return copy;
}

JsonValue json_value_string(char* string) {
    JsonValue value = {0};
    value.type = JSON_STRING;
    value.as.string = string;
    return value;
}

JsonValue json_value_number(double number) {
    JsonValue value = {0};
    value.type = JSON_NUMBER;
    value.as.number = number;
    return value;
}

JsonValue json_value_boolean(bool boolean) {
    JsonValue value = {0};
    value.type = JSON_BOOLEAN;
    value.as.number = boolean;
    return value;
}

JsonValue json_value_array(void) {
    JsonValue value = {0};
    value.type = JSON_ARRAY;
    return value;
}

JsonValue json_value_object(void) {
    JsonValue value = {0};
    value.type = JSON_OBJECT;
    return value;
}

void json_array_append(JsonArray* array, JsonValue value) {
    array->len++;
    
    if (array->next == NULL) {
        array->next = json_alloc(sizeof(*array->next));
        array->next->value = value;
        array->next->next = NULL;
    } else {
        JsonArrayNode* cur = array->next;
        while (cur != NULL && cur->next != NULL) cur = cur->next;

        cur->next = json_alloc(sizeof(*cur->next));
        cur->next->value = value;
        cur->next->next = NULL;
    }
}

void json_object_append(JsonObject* object, char* key, JsonValue value) {
    object->len++;

    if (object->next == NULL) {
        object->next = json_alloc(sizeof(*object->next));
        object->next->key = key;
        object->next->value = value;
        object->next->next = NULL;
    } else {
        JsonObjectItem* cur = object->next;
        while (cur != NULL && cur->next != NULL) cur = cur->next;

        cur->next = json_alloc(sizeof(*cur->next));
        cur->next->key = key;
        cur->next->value = value;
        cur->next->next = NULL;
    }
}

JsonValue* json_object_get_item(JsonObject* object, size_t index) {
    if (index > object->len) return NULL;

    JsonObjectItem* cur = object->next;
    while (cur != NULL && index > 0) {
        cur = cur->next;
        index--;
    }

    return &cur->value;
}

JsonValue* json_array_get_item(JsonArray* array, size_t index) {
    if (index > array->len) return NULL;

    JsonArrayNode* cur = array->next;
    while (cur != NULL && index > 0) {
        cur = cur->next;
        index--;
    }

    return &cur->value;
}

JsonLexer json_lexer(const char* content_start, size_t content_size) {
    JsonLexer lexer = { content_start, content_size, content_start };
    return lexer;
}

bool json_is_empty(JsonLexer* lexer) {
    return lexer->content_start >= lexer->content_start + lexer->content_len;
}

void json_lexer_trim_left(JsonLexer* lexer) {
    if (json_is_empty(lexer)) return;
    while (!json_is_empty(lexer) && isspace(*lexer->cursor)) lexer->cursor++;
}

bool json_lexer_expect_char(JsonLexer* lexer, char c) {
    if (json_is_empty(lexer)) {
        fprintf(stderr, "Unexpected EOF\n");
        return false;
    }
    if (*lexer->cursor != c) {
        fprintf(stderr, "Unexpected char '%c'\n", c);
        return false;
    }
    lexer->cursor++;
    return true;
}

bool json_lexer_parse_object_item(JsonLexer* lexer, JsonObjectItem* item);

bool json_lexer_parse_value(JsonLexer* lexer, JsonValue* value) {
    json_lexer_trim_left(lexer);

    if (isdigit(*lexer->cursor) || *lexer->cursor == '.') {
        value->type = JSON_NUMBER;

        char* endptr;
        value->as.number = strtod(lexer->cursor, &endptr);

        lexer->cursor = endptr;
        return true;
    }

    if (*lexer->cursor == '"') {
        value->type = JSON_STRING;

        const char* start = ++lexer->cursor;
        while (!json_is_empty(lexer) && *lexer->cursor != '"') {
            lexer->cursor++;
        }
        value->type = JSON_STRING;
        value->as.string = json_strndup(start, (int)(lexer->cursor - start));
        if (!json_lexer_expect_char(lexer, '"')) return false;

        return true;
    }

    if (*lexer->cursor == 'f' || *lexer->cursor == 't') {
        size_t stamp = json_arena_stamp();
        value->type = JSON_BOOLEAN;

        const char* start = lexer->cursor;
        while (!json_is_empty(lexer) && !isalpha(*lexer->cursor)) {
            lexer->cursor++;
        }
        char* value_ = json_strndup(start, (int)(lexer->cursor - start));

        value->type = JSON_STRING;
        value->as.boolean = strncmp(value_, "true", lexer->cursor - start) == 0;

        json_arena_rewind(stamp);
        return true;
    }

    if (*lexer->cursor == '{') {
        value->type = JSON_OBJECT;
        value->as.object.len = 0;
        value->as.object.next = NULL;

        json_lexer_expect_char(lexer, '{');
        while (!json_is_empty(lexer)) {
            JsonObjectItem item;
            if (!json_lexer_parse_object_item(lexer, &item)) return false;
            json_object_append(&value->as.object, item.key, item.value);
            if (*lexer->cursor != ',') break;
            if (!json_lexer_expect_char(lexer, ',')) return false;
        }
        json_lexer_trim_left(lexer);
        if (!json_lexer_expect_char(lexer, '}')) return false;
        return true;
    }

    if (*lexer->cursor == '[') {
        value->type = JSON_ARRAY;
        value->as.array.len = 0;
        value->as.array.next = NULL;

        if (!json_lexer_expect_char(lexer, '[')) return false;
        while (!json_is_empty(lexer)) {
            JsonValue value_;
            if (!json_lexer_parse_value(lexer, &value_)) return false;
            json_array_append(&value->as.array, value_);
            if (*lexer->cursor != ',') break;
            if (!json_lexer_expect_char(lexer, ',')) return false;
        }
        json_lexer_trim_left(lexer);
        if (!json_lexer_expect_char(lexer, ']')) return false;
        return true;
    }

    fprintf(stderr, "Unexpected char '%c'\n", *lexer->cursor);
    return false;
}

bool json_lexer_parse_object_item(JsonLexer* lexer, JsonObjectItem* item) {
    json_lexer_trim_left(lexer);
    if (!json_lexer_expect_char(lexer, '"')) return false;
    const char* start = lexer->cursor;
    while (!json_is_empty(lexer) && *lexer->cursor != '"') {
        lexer->cursor++;
    }
    item->key = json_strndup(start, (int)(lexer->cursor - start));
    if (!json_lexer_expect_char(lexer, '"')) return false;
    if (!json_lexer_expect_char(lexer, ':')) return false;
    if (!json_lexer_parse_value(lexer, &item->value)) return false;
    return true;
}

void json_stringify(JsonValue* value) {
    switch (value->type) {
        case JSON_NUMBER:
            printf("%.2lf", value->as.number);
            break;
        case JSON_STRING:
            printf("\"%s\"", value->as.string);
            break;
        case JSON_BOOLEAN:
            printf("%s", value->as.boolean ? "true" : "false");
            break;
        case JSON_ARRAY: {
            printf("[");
            JsonArrayNode* item = value->as.array.next;
            while (item != NULL) {
                json_stringify(&item->value);
                item = item->next;
                if (item != NULL) printf(", ");
            }
            printf("]");
        } break;
        case JSON_OBJECT: {
            printf("{");
            JsonObjectItem* item = value->as.object.next;
            while (item != NULL) {
                printf("\"%s\": ", item->key);
                json_stringify(&item->value);
                item = item->next;
                if (item != NULL) printf(", ");
            }
            printf("}");
        }break;
        default:
            UNREACHABLE();
    }
}

double* json_value_as_number(JsonValue* value) {
    if (value->type != JSON_NUMBER) return NULL;
    return &value->as.number;
}

char** json_value_as_string(JsonValue* value) {
    if (value->type != JSON_STRING) return NULL;
    return &value->as.string;
}

bool* json_value_as_boolean(JsonValue* value) {
    if (value->type != JSON_BOOLEAN) return NULL;
    return &value->as.boolean;
}

JsonArray* json_value_as_array(JsonValue* value) {
    if (value->type != JSON_ARRAY) return NULL;
    return &value->as.array;
}

JsonObject* json_value_as_object(JsonValue* value) {
    if (value->type != JSON_OBJECT) return NULL;
    return &value->as.object;
}

JsonValue* json_object_find_value(JsonObject* object, char* key) {
    if (object->len == 0) return NULL;

    JsonObjectItem* item = object->next;
    while (item != NULL) {
        if (strcmp(item->key, key) == 0) return &item->value;
        item = item->next;
    }

    return NULL;
}

#endif // JSON_IMPLEMENTATION
