
enum luaSymbol
{
    lua_SYMBOL_NULL,
    lua_SYMBOL_dot_start, 
    lua_SYMBOL_dot_end, 
    lua_SYMBOL_dot_error_terminal, 
    lua_SYMBOL_document, 
    lua_SYMBOL_element, 
    lua_SYMBOL_begin_element, 
    lua_SYMBOL_contents, 
    lua_SYMBOL_end_element, 
    lua_SYMBOL_eq_terminal, 
    lua_SYMBOL_left_curly_brace_terminal, 
    lua_SYMBOL_right_curly_brace_terminal, 
    lua_SYMBOL_comma_terminal, 
    lua_SYMBOL_content, 
    lua_SYMBOL_attribute, 
    lua_SYMBOL_boolean, 
    lua_SYMBOL_name, 
    lua_SYMBOL_nil, 
    lua_SYMBOL_true_terminal, 
    lua_SYMBOL_false_terminal, 
    lua_SYMBOL_integer, 
    lua_SYMBOL_real, 
    lua_SYMBOL_string, 
    lua_SYMBOL_COUNT
};


const sweet::parser::LexerAction lua_lexer_actions[] = 
{
    { 0, "string" },
    { 0, NULL }
};


extern const sweet::parser::LexerState lua_lexer_state_machine_state_0;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_1;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_2;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_3;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_4;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_5;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_6;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_7;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_8;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_9;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_10;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_11;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_12;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_13;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_14;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_15;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_16;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_17;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_18;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_19;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_20;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_21;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_22;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_23;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_24;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_25;
extern const sweet::parser::LexerState lua_lexer_state_machine_state_26;


const sweet::parser::LexerTransition lua_lexer_state_machine_state_0_transitions[] =
{
    { 34, 35, &lua_lexer_state_machine_state_25, NULL }, 
    { 39, 40, &lua_lexer_state_machine_state_25, NULL }, 
    { 43, 44, &lua_lexer_state_machine_state_19, NULL }, 
    { 44, 45, &lua_lexer_state_machine_state_4, NULL }, 
    { 45, 46, &lua_lexer_state_machine_state_19, NULL }, 
    { 48, 58, &lua_lexer_state_machine_state_18, NULL }, 
    { 61, 62, &lua_lexer_state_machine_state_1, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 102, &lua_lexer_state_machine_state_5, NULL }, 
    { 102, 103, &lua_lexer_state_machine_state_13, NULL }, 
    { 103, 110, &lua_lexer_state_machine_state_5, NULL }, 
    { 110, 111, &lua_lexer_state_machine_state_6, NULL }, 
    { 111, 116, &lua_lexer_state_machine_state_5, NULL }, 
    { 116, 117, &lua_lexer_state_machine_state_9, NULL }, 
    { 117, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 123, 124, &lua_lexer_state_machine_state_2, NULL }, 
    { 125, 126, &lua_lexer_state_machine_state_3, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_0 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_0_transitions,
    lua_lexer_state_machine_state_0_transitions + 18
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_1_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_1 =
{
    lua_SYMBOL_eq_terminal,
    lua_lexer_state_machine_state_1_transitions,
    lua_lexer_state_machine_state_1_transitions + 0
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_2_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_2 =
{
    lua_SYMBOL_left_curly_brace_terminal,
    lua_lexer_state_machine_state_2_transitions,
    lua_lexer_state_machine_state_2_transitions + 0
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_3_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_3 =
{
    lua_SYMBOL_right_curly_brace_terminal,
    lua_lexer_state_machine_state_3_transitions,
    lua_lexer_state_machine_state_3_transitions + 0
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_4_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_4 =
{
    lua_SYMBOL_comma_terminal,
    lua_lexer_state_machine_state_4_transitions,
    lua_lexer_state_machine_state_4_transitions + 0
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_5_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_5 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_5_transitions,
    lua_lexer_state_machine_state_5_transitions + 4
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_6_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 105, &lua_lexer_state_machine_state_5, NULL }, 
    { 105, 106, &lua_lexer_state_machine_state_7, NULL }, 
    { 106, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_6 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_6_transitions,
    lua_lexer_state_machine_state_6_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_7_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 108, &lua_lexer_state_machine_state_5, NULL }, 
    { 108, 109, &lua_lexer_state_machine_state_8, NULL }, 
    { 109, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_7 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_7_transitions,
    lua_lexer_state_machine_state_7_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_8_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_8 =
{
    lua_SYMBOL_nil,
    lua_lexer_state_machine_state_8_transitions,
    lua_lexer_state_machine_state_8_transitions + 4
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_9_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 114, &lua_lexer_state_machine_state_5, NULL }, 
    { 114, 115, &lua_lexer_state_machine_state_10, NULL }, 
    { 115, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_9 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_9_transitions,
    lua_lexer_state_machine_state_9_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_10_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 117, &lua_lexer_state_machine_state_5, NULL }, 
    { 117, 118, &lua_lexer_state_machine_state_11, NULL }, 
    { 118, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_10 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_10_transitions,
    lua_lexer_state_machine_state_10_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_11_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 101, &lua_lexer_state_machine_state_5, NULL }, 
    { 101, 102, &lua_lexer_state_machine_state_12, NULL }, 
    { 102, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_11 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_11_transitions,
    lua_lexer_state_machine_state_11_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_12_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_12 =
{
    lua_SYMBOL_true_terminal,
    lua_lexer_state_machine_state_12_transitions,
    lua_lexer_state_machine_state_12_transitions + 4
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_13_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 98, &lua_lexer_state_machine_state_14, NULL }, 
    { 98, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_13 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_13_transitions,
    lua_lexer_state_machine_state_13_transitions + 5
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_14_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 108, &lua_lexer_state_machine_state_5, NULL }, 
    { 108, 109, &lua_lexer_state_machine_state_15, NULL }, 
    { 109, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_14 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_14_transitions,
    lua_lexer_state_machine_state_14_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_15_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 115, &lua_lexer_state_machine_state_5, NULL }, 
    { 115, 116, &lua_lexer_state_machine_state_16, NULL }, 
    { 116, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_15 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_15_transitions,
    lua_lexer_state_machine_state_15_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_16_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 101, &lua_lexer_state_machine_state_5, NULL }, 
    { 101, 102, &lua_lexer_state_machine_state_17, NULL }, 
    { 102, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_16 =
{
    lua_SYMBOL_name,
    lua_lexer_state_machine_state_16_transitions,
    lua_lexer_state_machine_state_16_transitions + 6
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_17_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_5, NULL }, 
    { 65, 91, &lua_lexer_state_machine_state_5, NULL }, 
    { 95, 96, &lua_lexer_state_machine_state_5, NULL }, 
    { 97, 123, &lua_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_17 =
{
    lua_SYMBOL_false_terminal,
    lua_lexer_state_machine_state_17_transitions,
    lua_lexer_state_machine_state_17_transitions + 4
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_18_transitions[] =
{
    { 46, 47, &lua_lexer_state_machine_state_20, NULL }, 
    { 48, 58, &lua_lexer_state_machine_state_18, NULL }, 
    { 69, 70, &lua_lexer_state_machine_state_22, NULL }, 
    { 101, 102, &lua_lexer_state_machine_state_22, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_18 =
{
    lua_SYMBOL_integer,
    lua_lexer_state_machine_state_18_transitions,
    lua_lexer_state_machine_state_18_transitions + 4
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_19_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_18, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_19 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_19_transitions,
    lua_lexer_state_machine_state_19_transitions + 1
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_20_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_21, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_20 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_20_transitions,
    lua_lexer_state_machine_state_20_transitions + 1
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_21_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_21, NULL }, 
    { 69, 70, &lua_lexer_state_machine_state_22, NULL }, 
    { 101, 102, &lua_lexer_state_machine_state_22, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_21 =
{
    lua_SYMBOL_real,
    lua_lexer_state_machine_state_21_transitions,
    lua_lexer_state_machine_state_21_transitions + 3
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_22_transitions[] =
{
    { 43, 44, &lua_lexer_state_machine_state_23, NULL }, 
    { 45, 46, &lua_lexer_state_machine_state_23, NULL }, 
    { 48, 58, &lua_lexer_state_machine_state_24, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_22 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_22_transitions,
    lua_lexer_state_machine_state_22_transitions + 3
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_23_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_24, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_23 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_23_transitions,
    lua_lexer_state_machine_state_23_transitions + 1
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_24_transitions[] =
{
    { 48, 58, &lua_lexer_state_machine_state_24, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_24 =
{
    lua_SYMBOL_real,
    lua_lexer_state_machine_state_24_transitions,
    lua_lexer_state_machine_state_24_transitions + 1
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_25_transitions[] =
{
    { 0, 2147483647, &lua_lexer_state_machine_state_26, &lua_lexer_actions[0] }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_25 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_25_transitions,
    lua_lexer_state_machine_state_25_transitions + 1
};


const sweet::parser::LexerTransition lua_lexer_state_machine_state_26_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_26 =
{
    lua_SYMBOL_string,
    lua_lexer_state_machine_state_26_transitions,
    lua_lexer_state_machine_state_26_transitions + 0
};


extern const sweet::parser::LexerState lua_lexer_state_machine_state_27;


const sweet::parser::LexerTransition lua_lexer_state_machine_state_27_transitions[] =
{
    { 9, 11, &lua_lexer_state_machine_state_27, NULL }, 
    { 13, 14, &lua_lexer_state_machine_state_27, NULL }, 
    { 32, 33, &lua_lexer_state_machine_state_27, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState lua_lexer_state_machine_state_27 =
{
    lua_SYMBOL_NULL,
    lua_lexer_state_machine_state_27_transitions,
    lua_lexer_state_machine_state_27_transitions + 3
};


const sweet::parser::LexerStateMachine lua_lexer_state_machine = 
{
    &lua_lexer_state_machine_state_0, 
    &lua_lexer_state_machine_state_27, 
    lua_lexer_actions,
    lua_lexer_actions + 1
};


const sweet::parser::ParserAction lua_parser_actions[] = 
{
    { 0, "begin_element" },
    { 1, "end_element" },
    { 2, "nil_attribute" },
    { 3, "boolean_attribute" },
    { 4, "integer_attribute" },
    { 5, "real_attribute" },
    { 6, "string_attribute" },
    { 0, NULL }
};


const sweet::parser::ParserProduction lua_parser_production_0 =
{
    lua_SYMBOL_dot_start, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_1 =
{
    lua_SYMBOL_document, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_2 =
{
    lua_SYMBOL_element, 
    NULL, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_3 =
{
    lua_SYMBOL_begin_element, 
    lua_parser_actions + 0, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_4 =
{
    lua_SYMBOL_end_element, 
    lua_parser_actions + 1, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_5 =
{
    lua_SYMBOL_contents, 
    NULL, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_6 =
{
    lua_SYMBOL_contents, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_7 =
{
    lua_SYMBOL_content, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_8 =
{
    lua_SYMBOL_content, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_9 =
{
    lua_SYMBOL_attribute, 
    lua_parser_actions + 2, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_10 =
{
    lua_SYMBOL_attribute, 
    lua_parser_actions + 3, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_11 =
{
    lua_SYMBOL_attribute, 
    lua_parser_actions + 4, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_12 =
{
    lua_SYMBOL_attribute, 
    lua_parser_actions + 5, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_13 =
{
    lua_SYMBOL_attribute, 
    lua_parser_actions + 6, 
    3
};


const sweet::parser::ParserProduction lua_parser_production_14 =
{
    lua_SYMBOL_name, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_15 =
{
    lua_SYMBOL_nil, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_16 =
{
    lua_SYMBOL_boolean, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_17 =
{
    lua_SYMBOL_boolean, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_18 =
{
    lua_SYMBOL_integer, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_19 =
{
    lua_SYMBOL_real, 
    NULL, 
    1
};


const sweet::parser::ParserProduction lua_parser_production_20 =
{
    lua_SYMBOL_string, 
    NULL, 
    1
};


extern const sweet::parser::ParserState lua_parser_state_0;
extern const sweet::parser::ParserState lua_parser_state_1;
extern const sweet::parser::ParserState lua_parser_state_2;
extern const sweet::parser::ParserState lua_parser_state_3;
extern const sweet::parser::ParserState lua_parser_state_4;
extern const sweet::parser::ParserState lua_parser_state_5;
extern const sweet::parser::ParserState lua_parser_state_6;
extern const sweet::parser::ParserState lua_parser_state_7;
extern const sweet::parser::ParserState lua_parser_state_8;
extern const sweet::parser::ParserState lua_parser_state_9;
extern const sweet::parser::ParserState lua_parser_state_10;
extern const sweet::parser::ParserState lua_parser_state_11;
extern const sweet::parser::ParserState lua_parser_state_12;
extern const sweet::parser::ParserState lua_parser_state_13;
extern const sweet::parser::ParserState lua_parser_state_14;
extern const sweet::parser::ParserState lua_parser_state_15;
extern const sweet::parser::ParserState lua_parser_state_16;
extern const sweet::parser::ParserState lua_parser_state_17;
extern const sweet::parser::ParserState lua_parser_state_18;
extern const sweet::parser::ParserState lua_parser_state_19;
extern const sweet::parser::ParserState lua_parser_state_20;
extern const sweet::parser::ParserState lua_parser_state_21;
extern const sweet::parser::ParserState lua_parser_state_22;
extern const sweet::parser::ParserState lua_parser_state_23;


const sweet::parser::ParserTransition lua_parser_state_0_transitions[] = 
{
    { lua_SYMBOL_begin_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_3, NULL },
    { lua_SYMBOL_document, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_1, NULL },
    { lua_SYMBOL_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_2, NULL },
    { lua_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_7, NULL },
};


const sweet::parser::ParserState lua_parser_state_0 =
{
    lua_parser_state_0_transitions,
    lua_parser_state_0_transitions + 4
};


const sweet::parser::ParserTransition lua_parser_state_1_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_0 },
};


const sweet::parser::ParserState lua_parser_state_1 =
{
    lua_parser_state_1_transitions,
    lua_parser_state_1_transitions + 1
};


const sweet::parser::ParserTransition lua_parser_state_2_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_1 },
};


const sweet::parser::ParserState lua_parser_state_2 =
{
    lua_parser_state_2_transitions,
    lua_parser_state_2_transitions + 1
};


const sweet::parser::ParserTransition lua_parser_state_3_transitions[] = 
{
    { lua_SYMBOL_begin_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_3, NULL },
    { lua_SYMBOL_contents, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_5, NULL },
    { lua_SYMBOL_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_15, NULL },
    { lua_SYMBOL_content, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_14, NULL },
    { lua_SYMBOL_attribute, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_16, NULL },
    { lua_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_8, NULL },
};


const sweet::parser::ParserState lua_parser_state_3 =
{
    lua_parser_state_3_transitions,
    lua_parser_state_3_transitions + 6
};


const sweet::parser::ParserTransition lua_parser_state_4_transitions[] = 
{
    { lua_SYMBOL_begin_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_3, NULL },
    { lua_SYMBOL_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_15, NULL },
    { lua_SYMBOL_content, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_13, NULL },
    { lua_SYMBOL_attribute, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_16, NULL },
    { lua_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_8, NULL },
};


const sweet::parser::ParserState lua_parser_state_4 =
{
    lua_parser_state_4_transitions,
    lua_parser_state_4_transitions + 5
};


const sweet::parser::ParserTransition lua_parser_state_5_transitions[] = 
{
    { lua_SYMBOL_end_element, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_6, NULL },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_12, NULL },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_4, NULL },
};


const sweet::parser::ParserState lua_parser_state_5 =
{
    lua_parser_state_5_transitions,
    lua_parser_state_5_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_6_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_2 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_2 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_2 },
};


const sweet::parser::ParserState lua_parser_state_6 =
{
    lua_parser_state_6_transitions,
    lua_parser_state_6_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_7_transitions[] = 
{
    { lua_SYMBOL_eq_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_9, NULL },
};


const sweet::parser::ParserState lua_parser_state_7 =
{
    lua_parser_state_7_transitions,
    lua_parser_state_7_transitions + 1
};


const sweet::parser::ParserTransition lua_parser_state_8_transitions[] = 
{
    { lua_SYMBOL_eq_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_10, NULL },
};


const sweet::parser::ParserState lua_parser_state_8 =
{
    lua_parser_state_8_transitions,
    lua_parser_state_8_transitions + 1
};


const sweet::parser::ParserTransition lua_parser_state_9_transitions[] = 
{
    { lua_SYMBOL_left_curly_brace_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_11, NULL },
};


const sweet::parser::ParserState lua_parser_state_9 =
{
    lua_parser_state_9_transitions,
    lua_parser_state_9_transitions + 1
};


const sweet::parser::ParserTransition lua_parser_state_10_transitions[] = 
{
    { lua_SYMBOL_left_curly_brace_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_11, NULL },
    { lua_SYMBOL_boolean, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_18, NULL },
    { lua_SYMBOL_nil, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_17, NULL },
    { lua_SYMBOL_true_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_22, NULL },
    { lua_SYMBOL_false_terminal, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_23, NULL },
    { lua_SYMBOL_integer, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_19, NULL },
    { lua_SYMBOL_real, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_20, NULL },
    { lua_SYMBOL_string, sweet::parser::TRANSITION_SHIFT, &lua_parser_state_21, NULL },
};


const sweet::parser::ParserState lua_parser_state_10 =
{
    lua_parser_state_10_transitions,
    lua_parser_state_10_transitions + 8
};


const sweet::parser::ParserTransition lua_parser_state_11_transitions[] = 
{
    { lua_SYMBOL_name, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_3 },
};


const sweet::parser::ParserState lua_parser_state_11 =
{
    lua_parser_state_11_transitions,
    lua_parser_state_11_transitions + 1
};


const sweet::parser::ParserTransition lua_parser_state_12_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_4 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_4 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_4 },
};


const sweet::parser::ParserState lua_parser_state_12 =
{
    lua_parser_state_12_transitions,
    lua_parser_state_12_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_13_transitions[] = 
{
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_5 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_5 },
};


const sweet::parser::ParserState lua_parser_state_13 =
{
    lua_parser_state_13_transitions,
    lua_parser_state_13_transitions + 2
};


const sweet::parser::ParserTransition lua_parser_state_14_transitions[] = 
{
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_6 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_6 },
};


const sweet::parser::ParserState lua_parser_state_14 =
{
    lua_parser_state_14_transitions,
    lua_parser_state_14_transitions + 2
};


const sweet::parser::ParserTransition lua_parser_state_15_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_7 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_7 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_7 },
};


const sweet::parser::ParserState lua_parser_state_15 =
{
    lua_parser_state_15_transitions,
    lua_parser_state_15_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_16_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_8 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_8 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_8 },
};


const sweet::parser::ParserState lua_parser_state_16 =
{
    lua_parser_state_16_transitions,
    lua_parser_state_16_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_17_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_9 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_9 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_9 },
};


const sweet::parser::ParserState lua_parser_state_17 =
{
    lua_parser_state_17_transitions,
    lua_parser_state_17_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_18_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_10 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_10 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_10 },
};


const sweet::parser::ParserState lua_parser_state_18 =
{
    lua_parser_state_18_transitions,
    lua_parser_state_18_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_19_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_11 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_11 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_11 },
};


const sweet::parser::ParserState lua_parser_state_19 =
{
    lua_parser_state_19_transitions,
    lua_parser_state_19_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_20_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_12 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_12 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_12 },
};


const sweet::parser::ParserState lua_parser_state_20 =
{
    lua_parser_state_20_transitions,
    lua_parser_state_20_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_21_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_13 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_13 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_13 },
};


const sweet::parser::ParserState lua_parser_state_21 =
{
    lua_parser_state_21_transitions,
    lua_parser_state_21_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_22_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_16 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_16 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_16 },
};


const sweet::parser::ParserState lua_parser_state_22 =
{
    lua_parser_state_22_transitions,
    lua_parser_state_22_transitions + 3
};


const sweet::parser::ParserTransition lua_parser_state_23_transitions[] = 
{
    { lua_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_17 },
    { lua_SYMBOL_right_curly_brace_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_17 },
    { lua_SYMBOL_comma_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &lua_parser_production_17 },
};


const sweet::parser::ParserState lua_parser_state_23 =
{
    lua_parser_state_23_transitions,
    lua_parser_state_23_transitions + 3
};


sweet::parser::ParserStateMachine lua_parser_state_machine = 
{
    &lua_parser_state_0,
    &lua_lexer_state_machine,
    lua_SYMBOL_dot_start,
    lua_SYMBOL_dot_end,
    lua_SYMBOL_dot_error_terminal,
    lua_parser_actions,
    lua_parser_actions + 7,
};

