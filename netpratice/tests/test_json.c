/* ************************************************************************** */
/*                                                                            */
/*   test_json.c                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jojo8356 <jojo8356@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Tests : parseur JSON (objets, tableaux, strings, échappements).      */
/* ************************************************************************** */

# include "test_framework.h"

TEST(test_parse_object)
{
    t_json            *j = json_parse("{\"a\":1,\"b\":\"x\"}");
    ASSERT(j != NULL, "objet parsé");
    ASSERT_EQ_INT(j->type, JSON_OBJECT, "type objet");
    ASSERT_EQ_INT(j->count, 2, "2 clés");
    {
        t_json            *a = json_get(j, "a");
        t_json            *b = json_get(j, "b");
        ASSERT(a != NULL && a->type == JSON_NUMBER, "a nombre");
        ASSERT_EQ_INT((int)a->num, 1, "a=1");
        ASSERT(b != NULL && b->type == JSON_STRING, "b string");
        ASSERT_EQ_STR(b->str, "x", "b=x");
    }
    json_free(j);
}

TEST(test_parse_array)
{
    t_json            *j = json_parse("[1,2,3]");
    ASSERT(j != NULL, "tableau parsé");
    ASSERT_EQ_INT(j->type, JSON_ARRAY, "type tableau");
    ASSERT_EQ_INT(j->count, 3, "3 éléments");
    ASSERT_EQ_INT((int)json_at(j, 0)->num, 1, "el0");
    ASSERT_EQ_INT((int)json_at(j, 2)->num, 3, "el2");
    ASSERT(json_at(j, 5) == NULL, "hors limites");
    json_free(j);
}

TEST(test_parse_empty)
{
    t_json            *j = json_parse("{}");
    ASSERT(j != NULL && j->count == 0, "objet vide");
    json_free(j);
    j = json_parse("[]");
    ASSERT(j != NULL && j->count == 0, "tableau vide");
    json_free(j);
}

TEST(test_parse_escapes)
{
    t_json            *j = json_parse("\"a\\nb\\tc\"");
    ASSERT(j != NULL && j->type == JSON_STRING, "string");
    ASSERT_EQ_STR(j->str, "a\nb\tc", "échappements");
    json_free(j);
}

TEST(test_parse_literals)
{
    t_json            *t = json_parse("true");
    ASSERT(t != NULL && t->type == JSON_BOOL && t->boolean == 1, "true");
    json_free(t);
    t = json_parse("false");
    ASSERT(t != NULL && t->type == JSON_BOOL && t->boolean == 0, "false");
    json_free(t);
    t = json_parse("null");
    ASSERT(t != NULL && t->type == JSON_NULL, "null");
    json_free(t);
}

TEST(test_parse_nested)
{
    t_json            *j = json_parse("{\"r\":{\"route\":\"0.0.0.0/0\",\"gate\":\"1.2.3.4\"}}");
    ASSERT(j != NULL, "parsé");
    {
        t_json            *r = json_get(j, "r");
        t_json            *route = json_get(r, "route");
        t_json            *gate = json_get(r, "gate");
        ASSERT_EQ_STR(route->str, "0.0.0.0/0", "route");
        ASSERT_EQ_STR(gate->str, "1.2.3.4", "gate");
    }
    json_free(j);
}

TEST(test_parse_invalid)
{
    ASSERT(json_parse("") == NULL, "vide invalide");
    ASSERT(json_parse("{") == NULL, "objet non fermé");
    ASSERT(json_parse("{\"a\":}") == NULL, "valeur manquante");
    ASSERT(json_parse("[1,]") == NULL, "tableau invalide");
}

int            main(void)
{
    RUN(test_parse_object);
    RUN(test_parse_array);
    RUN(test_parse_empty);
    RUN(test_parse_escapes);
    RUN(test_parse_literals);
    RUN(test_parse_nested);
    RUN(test_parse_invalid);
    SUMMARY();
}
