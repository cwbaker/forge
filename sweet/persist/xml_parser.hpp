
enum xmlSymbol
{
    xml_SYMBOL_NULL,
    xml_SYMBOL_dot_start, 
    xml_SYMBOL_dot_end, 
    xml_SYMBOL_dot_error_terminal, 
    xml_SYMBOL_lt_terminal, 
    xml_SYMBOL_gt_terminal, 
    xml_SYMBOL_document, 
    xml_SYMBOL_prolog, 
    xml_SYMBOL_element, 
    xml_SYMBOL_lt__question_xml_terminal, 
    xml_SYMBOL_attributes, 
    xml_SYMBOL_question__gt_terminal, 
    xml_SYMBOL_begin_element, 
    xml_SYMBOL_end_element, 
    xml_SYMBOL_slash__gt_terminal, 
    xml_SYMBOL_elements, 
    xml_SYMBOL_lt__slash_terminal, 
    xml_SYMBOL_name, 
    xml_SYMBOL_attribute, 
    xml_SYMBOL_eq_terminal, 
    xml_SYMBOL_value, 
    xml_SYMBOL_COUNT
};


const sweet::parser::LexerAction xml_lexer_actions[] = 
{
    { 0, "string" },
    { 0, NULL }
};


extern const sweet::parser::LexerState xml_lexer_state_machine_state_0;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_1;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_2;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_3;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_4;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_5;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_6;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_7;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_8;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_9;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_10;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_11;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_12;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_13;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_14;
extern const sweet::parser::LexerState xml_lexer_state_machine_state_15;


const sweet::parser::LexerTransition xml_lexer_state_machine_state_0_transitions[] =
{
    { 34, 35, &xml_lexer_state_machine_state_14, NULL }, 
    { 39, 40, &xml_lexer_state_machine_state_14, NULL }, 
    { 47, 48, &xml_lexer_state_machine_state_8, NULL }, 
    { 58, 59, &xml_lexer_state_machine_state_12, NULL }, 
    { 60, 61, &xml_lexer_state_machine_state_1, NULL }, 
    { 61, 62, &xml_lexer_state_machine_state_13, NULL }, 
    { 62, 63, &xml_lexer_state_machine_state_10, NULL }, 
    { 63, 64, &xml_lexer_state_machine_state_6, NULL }, 
    { 65, 91, &xml_lexer_state_machine_state_12, NULL }, 
    { 95, 96, &xml_lexer_state_machine_state_12, NULL }, 
    { 97, 123, &xml_lexer_state_machine_state_12, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_0 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_0_transitions,
    xml_lexer_state_machine_state_0_transitions + 11
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_1_transitions[] =
{
    { 47, 48, &xml_lexer_state_machine_state_11, NULL }, 
    { 63, 64, &xml_lexer_state_machine_state_2, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_1 =
{
    xml_SYMBOL_lt_terminal,
    xml_lexer_state_machine_state_1_transitions,
    xml_lexer_state_machine_state_1_transitions + 2
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_2_transitions[] =
{
    { 120, 121, &xml_lexer_state_machine_state_3, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_2 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_2_transitions,
    xml_lexer_state_machine_state_2_transitions + 1
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_3_transitions[] =
{
    { 109, 110, &xml_lexer_state_machine_state_4, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_3 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_3_transitions,
    xml_lexer_state_machine_state_3_transitions + 1
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_4_transitions[] =
{
    { 108, 109, &xml_lexer_state_machine_state_5, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_4 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_4_transitions,
    xml_lexer_state_machine_state_4_transitions + 1
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_5_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_5 =
{
    xml_SYMBOL_lt__question_xml_terminal,
    xml_lexer_state_machine_state_5_transitions,
    xml_lexer_state_machine_state_5_transitions + 0
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_6_transitions[] =
{
    { 62, 63, &xml_lexer_state_machine_state_7, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_6 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_6_transitions,
    xml_lexer_state_machine_state_6_transitions + 1
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_7_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_7 =
{
    xml_SYMBOL_question__gt_terminal,
    xml_lexer_state_machine_state_7_transitions,
    xml_lexer_state_machine_state_7_transitions + 0
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_8_transitions[] =
{
    { 62, 63, &xml_lexer_state_machine_state_9, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_8 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_8_transitions,
    xml_lexer_state_machine_state_8_transitions + 1
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_9_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_9 =
{
    xml_SYMBOL_slash__gt_terminal,
    xml_lexer_state_machine_state_9_transitions,
    xml_lexer_state_machine_state_9_transitions + 0
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_10_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_10 =
{
    xml_SYMBOL_gt_terminal,
    xml_lexer_state_machine_state_10_transitions,
    xml_lexer_state_machine_state_10_transitions + 0
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_11_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_11 =
{
    xml_SYMBOL_lt__slash_terminal,
    xml_lexer_state_machine_state_11_transitions,
    xml_lexer_state_machine_state_11_transitions + 0
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_12_transitions[] =
{
    { 45, 47, &xml_lexer_state_machine_state_12, NULL }, 
    { 48, 59, &xml_lexer_state_machine_state_12, NULL }, 
    { 65, 91, &xml_lexer_state_machine_state_12, NULL }, 
    { 92, 93, &xml_lexer_state_machine_state_12, NULL }, 
    { 95, 96, &xml_lexer_state_machine_state_12, NULL }, 
    { 97, 123, &xml_lexer_state_machine_state_12, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_12 =
{
    xml_SYMBOL_name,
    xml_lexer_state_machine_state_12_transitions,
    xml_lexer_state_machine_state_12_transitions + 6
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_13_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_13 =
{
    xml_SYMBOL_eq_terminal,
    xml_lexer_state_machine_state_13_transitions,
    xml_lexer_state_machine_state_13_transitions + 0
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_14_transitions[] =
{
    { 0, 2147483647, &xml_lexer_state_machine_state_15, &xml_lexer_actions[0] }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_14 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_14_transitions,
    xml_lexer_state_machine_state_14_transitions + 1
};


const sweet::parser::LexerTransition xml_lexer_state_machine_state_15_transitions[] =
{
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_15 =
{
    xml_SYMBOL_value,
    xml_lexer_state_machine_state_15_transitions,
    xml_lexer_state_machine_state_15_transitions + 0
};


extern const sweet::parser::LexerState xml_lexer_state_machine_state_16;


const sweet::parser::LexerTransition xml_lexer_state_machine_state_16_transitions[] =
{
    { 9, 11, &xml_lexer_state_machine_state_16, NULL }, 
    { 13, 14, &xml_lexer_state_machine_state_16, NULL }, 
    { 32, 33, &xml_lexer_state_machine_state_16, NULL }, 
    { 0, 0, NULL }
};


const sweet::parser::LexerState xml_lexer_state_machine_state_16 =
{
    xml_SYMBOL_NULL,
    xml_lexer_state_machine_state_16_transitions,
    xml_lexer_state_machine_state_16_transitions + 3
};


const sweet::parser::LexerStateMachine xml_lexer_state_machine = 
{
    &xml_lexer_state_machine_state_0, 
    &xml_lexer_state_machine_state_16, 
    xml_lexer_actions,
    xml_lexer_actions + 1
};


const sweet::parser::ParserAction xml_parser_actions[] = 
{
    { 0, "begin_element" },
    { 1, "end_element" },
    { 2, "attribute" },
    { 0, NULL }
};


const sweet::parser::ParserProduction xml_parser_production_0 =
{
    xml_SYMBOL_dot_start, 
    NULL, 
    1
};


const sweet::parser::ParserProduction xml_parser_production_1 =
{
    xml_SYMBOL_document, 
    NULL, 
    2
};


const sweet::parser::ParserProduction xml_parser_production_2 =
{
    xml_SYMBOL_prolog, 
    NULL, 
    3
};


const sweet::parser::ParserProduction xml_parser_production_3 =
{
    xml_SYMBOL_prolog, 
    NULL, 
    0
};


const sweet::parser::ParserProduction xml_parser_production_4 =
{
    xml_SYMBOL_element, 
    NULL, 
    3
};


const sweet::parser::ParserProduction xml_parser_production_5 =
{
    xml_SYMBOL_begin_element, 
    xml_parser_actions + 0, 
    2
};


const sweet::parser::ParserProduction xml_parser_production_6 =
{
    xml_SYMBOL_end_element, 
    xml_parser_actions + 1, 
    1
};


const sweet::parser::ParserProduction xml_parser_production_7 =
{
    xml_SYMBOL_end_element, 
    xml_parser_actions + 1, 
    5
};


const sweet::parser::ParserProduction xml_parser_production_8 =
{
    xml_SYMBOL_elements, 
    NULL, 
    2
};


const sweet::parser::ParserProduction xml_parser_production_9 =
{
    xml_SYMBOL_elements, 
    NULL, 
    1
};


const sweet::parser::ParserProduction xml_parser_production_10 =
{
    xml_SYMBOL_elements, 
    NULL, 
    0
};


const sweet::parser::ParserProduction xml_parser_production_11 =
{
    xml_SYMBOL_name, 
    NULL, 
    1
};


const sweet::parser::ParserProduction xml_parser_production_12 =
{
    xml_SYMBOL_attributes, 
    NULL, 
    2
};


const sweet::parser::ParserProduction xml_parser_production_13 =
{
    xml_SYMBOL_attributes, 
    NULL, 
    1
};


const sweet::parser::ParserProduction xml_parser_production_14 =
{
    xml_SYMBOL_attributes, 
    NULL, 
    0
};


const sweet::parser::ParserProduction xml_parser_production_15 =
{
    xml_SYMBOL_attribute, 
    xml_parser_actions + 2, 
    3
};


const sweet::parser::ParserProduction xml_parser_production_16 =
{
    xml_SYMBOL_value, 
    NULL, 
    1
};


extern const sweet::parser::ParserState xml_parser_state_0;
extern const sweet::parser::ParserState xml_parser_state_1;
extern const sweet::parser::ParserState xml_parser_state_2;
extern const sweet::parser::ParserState xml_parser_state_3;
extern const sweet::parser::ParserState xml_parser_state_4;
extern const sweet::parser::ParserState xml_parser_state_5;
extern const sweet::parser::ParserState xml_parser_state_6;
extern const sweet::parser::ParserState xml_parser_state_7;
extern const sweet::parser::ParserState xml_parser_state_8;
extern const sweet::parser::ParserState xml_parser_state_9;
extern const sweet::parser::ParserState xml_parser_state_10;
extern const sweet::parser::ParserState xml_parser_state_11;
extern const sweet::parser::ParserState xml_parser_state_12;
extern const sweet::parser::ParserState xml_parser_state_13;
extern const sweet::parser::ParserState xml_parser_state_14;
extern const sweet::parser::ParserState xml_parser_state_15;
extern const sweet::parser::ParserState xml_parser_state_16;
extern const sweet::parser::ParserState xml_parser_state_17;
extern const sweet::parser::ParserState xml_parser_state_18;
extern const sweet::parser::ParserState xml_parser_state_19;
extern const sweet::parser::ParserState xml_parser_state_20;
extern const sweet::parser::ParserState xml_parser_state_21;
extern const sweet::parser::ParserState xml_parser_state_22;
extern const sweet::parser::ParserState xml_parser_state_23;
extern const sweet::parser::ParserState xml_parser_state_24;

/*
0:
. lt__question_xml_terminal attributes question__gt_terminal ; lt_terminal
. ; lt_terminal
. document ; dot_end
. prolog element ; dot_end
reduce to prolog on lt_terminal ; 
shift to 23 on document ; 
shift to 6 on prolog ; 
shift to 1 on lt__question_xml_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_0_transitions[] = 
{
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_3 },
    { xml_SYMBOL_document, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_23, NULL },
    { xml_SYMBOL_prolog, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_6, NULL },
    { xml_SYMBOL_lt__question_xml_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_1, NULL },
};


const sweet::parser::ParserState xml_parser_state_0 =
{
    xml_parser_state_0_transitions,
    xml_parser_state_0_transitions + 4
};

/*
1:
lt__question_xml_terminal . attributes question__gt_terminal ; lt_terminal
. attributes attribute ; question__gt_terminal, name
. attribute ; question__gt_terminal, name
. ; question__gt_terminal, name
. name eq_terminal value ; question__gt_terminal, name
reduce to attributes on question__gt_terminal ; 
shift to 2 on attributes ; 
shift to 20 on name ; 
shift to 19 on attribute ; 

*/

const sweet::parser::ParserTransition xml_parser_state_1_transitions[] = 
{
    { xml_SYMBOL_question__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_14 },
    { xml_SYMBOL_attributes, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_2, NULL },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_20, NULL },
    { xml_SYMBOL_attribute, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_19, NULL },
};


const sweet::parser::ParserState xml_parser_state_1 =
{
    xml_parser_state_1_transitions,
    xml_parser_state_1_transitions + 4
};

/*
2:
lt__question_xml_terminal attributes . question__gt_terminal ; lt_terminal
attributes . attribute ; question__gt_terminal, name
. name eq_terminal value ; question__gt_terminal, name
shift to 3 on question__gt_terminal ; 
shift to 20 on name ; 
shift to 18 on attribute ; 

*/

const sweet::parser::ParserTransition xml_parser_state_2_transitions[] = 
{
    { xml_SYMBOL_question__gt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_3, NULL },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_20, NULL },
    { xml_SYMBOL_attribute, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_18, NULL },
};


const sweet::parser::ParserState xml_parser_state_2 =
{
    xml_parser_state_2_transitions,
    xml_parser_state_2_transitions + 3
};

/*
3:
lt__question_xml_terminal attributes question__gt_terminal . ; lt_terminal
reduce to prolog on lt_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_3_transitions[] = 
{
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_2 },
};


const sweet::parser::ParserState xml_parser_state_3 =
{
    xml_parser_state_3_transitions,
    xml_parser_state_3_transitions + 1
};

/*
4:
. begin_element attributes end_element ; lt_terminal, lt__slash_terminal
. lt_terminal name ; slash__gt_terminal, gt_terminal, name
gt_terminal . elements lt__slash_terminal name gt_terminal ; dot_end, lt_terminal, lt__slash_terminal
. elements element ; lt_terminal, lt__slash_terminal
. element ; lt_terminal, lt__slash_terminal
. ; lt_terminal, lt__slash_terminal
shift to 7 on begin_element ; 
shift to 5 on elements ; 
shift to 10 on lt_terminal ; 
shift to 17 on element ; 
reduce to elements on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_4_transitions[] = 
{
    { xml_SYMBOL_begin_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_7, NULL },
    { xml_SYMBOL_elements, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_5, NULL },
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_10, NULL },
    { xml_SYMBOL_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_17, NULL },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_10 },
};


const sweet::parser::ParserState xml_parser_state_4 =
{
    xml_parser_state_4_transitions,
    xml_parser_state_4_transitions + 5
};

/*
5:
. begin_element attributes end_element ; lt_terminal, lt__slash_terminal
. lt_terminal name ; slash__gt_terminal, gt_terminal, name
gt_terminal elements . lt__slash_terminal name gt_terminal ; dot_end, lt_terminal, lt__slash_terminal
elements . element ; lt_terminal, lt__slash_terminal
shift to 7 on begin_element ; 
shift to 10 on lt_terminal ; 
shift to 16 on element ; 
shift to 13 on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_5_transitions[] = 
{
    { xml_SYMBOL_begin_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_7, NULL },
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_10, NULL },
    { xml_SYMBOL_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_16, NULL },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_13, NULL },
};


const sweet::parser::ParserState xml_parser_state_5 =
{
    xml_parser_state_5_transitions,
    xml_parser_state_5_transitions + 4
};

/*
6:
. begin_element attributes end_element ; dot_end
. lt_terminal name ; slash__gt_terminal, gt_terminal, name
prolog . element ; dot_end
shift to 7 on begin_element ; 
shift to 10 on lt_terminal ; 
shift to 24 on element ; 

*/

const sweet::parser::ParserTransition xml_parser_state_6_transitions[] = 
{
    { xml_SYMBOL_begin_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_7, NULL },
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_10, NULL },
    { xml_SYMBOL_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_24, NULL },
};


const sweet::parser::ParserState xml_parser_state_6 =
{
    xml_parser_state_6_transitions,
    xml_parser_state_6_transitions + 3
};

/*
7:
begin_element . attributes end_element ; dot_end, lt_terminal, lt__slash_terminal
. attributes attribute ; slash__gt_terminal, gt_terminal, name
. attribute ; slash__gt_terminal, gt_terminal, name
. ; slash__gt_terminal, gt_terminal, name
. name eq_terminal value ; slash__gt_terminal, gt_terminal, name
reduce to attributes on slash__gt_terminal ; 
reduce to attributes on gt_terminal ; 
shift to 8 on attributes ; 
shift to 20 on name ; 
shift to 19 on attribute ; 

*/

const sweet::parser::ParserTransition xml_parser_state_7_transitions[] = 
{
    { xml_SYMBOL_slash__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_14 },
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_14 },
    { xml_SYMBOL_attributes, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_8, NULL },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_20, NULL },
    { xml_SYMBOL_attribute, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_19, NULL },
};


const sweet::parser::ParserState xml_parser_state_7 =
{
    xml_parser_state_7_transitions,
    xml_parser_state_7_transitions + 5
};

/*
8:
begin_element attributes . end_element ; dot_end, lt_terminal, lt__slash_terminal
. slash__gt_terminal ; dot_end, lt_terminal, lt__slash_terminal
. gt_terminal elements lt__slash_terminal name gt_terminal ; dot_end, lt_terminal, lt__slash_terminal
attributes . attribute ; slash__gt_terminal, gt_terminal, name
. name eq_terminal value ; slash__gt_terminal, gt_terminal, name
shift to 9 on end_element ; 
shift to 12 on slash__gt_terminal ; 
shift to 4 on gt_terminal ; 
shift to 20 on name ; 
shift to 18 on attribute ; 

*/

const sweet::parser::ParserTransition xml_parser_state_8_transitions[] = 
{
    { xml_SYMBOL_end_element, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_9, NULL },
    { xml_SYMBOL_slash__gt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_12, NULL },
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_4, NULL },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_20, NULL },
    { xml_SYMBOL_attribute, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_18, NULL },
};


const sweet::parser::ParserState xml_parser_state_8 =
{
    xml_parser_state_8_transitions,
    xml_parser_state_8_transitions + 5
};

/*
9:
begin_element attributes end_element . ; dot_end, lt_terminal, lt__slash_terminal
reduce to element on dot_end ; 
reduce to element on lt_terminal ; 
reduce to element on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_9_transitions[] = 
{
    { xml_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_4 },
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_4 },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_4 },
};


const sweet::parser::ParserState xml_parser_state_9 =
{
    xml_parser_state_9_transitions,
    xml_parser_state_9_transitions + 3
};

/*
10:
lt_terminal . name ; slash__gt_terminal, gt_terminal, name
shift to 11 on name ; 

*/

const sweet::parser::ParserTransition xml_parser_state_10_transitions[] = 
{
    { xml_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_11, NULL },
};


const sweet::parser::ParserState xml_parser_state_10 =
{
    xml_parser_state_10_transitions,
    xml_parser_state_10_transitions + 1
};

/*
11:
lt_terminal name . ; slash__gt_terminal, gt_terminal, name
reduce to begin_element on slash__gt_terminal ; 
reduce to begin_element on gt_terminal ; 
reduce to begin_element on name ; 

*/

const sweet::parser::ParserTransition xml_parser_state_11_transitions[] = 
{
    { xml_SYMBOL_slash__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_5 },
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_5 },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_5 },
};


const sweet::parser::ParserState xml_parser_state_11 =
{
    xml_parser_state_11_transitions,
    xml_parser_state_11_transitions + 3
};

/*
12:
slash__gt_terminal . ; dot_end, lt_terminal, lt__slash_terminal
reduce to end_element on dot_end ; 
reduce to end_element on lt_terminal ; 
reduce to end_element on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_12_transitions[] = 
{
    { xml_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_6 },
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_6 },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_6 },
};


const sweet::parser::ParserState xml_parser_state_12 =
{
    xml_parser_state_12_transitions,
    xml_parser_state_12_transitions + 3
};

/*
13:
gt_terminal elements lt__slash_terminal . name gt_terminal ; dot_end, lt_terminal, lt__slash_terminal
shift to 14 on name ; 

*/

const sweet::parser::ParserTransition xml_parser_state_13_transitions[] = 
{
    { xml_SYMBOL_name, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_14, NULL },
};


const sweet::parser::ParserState xml_parser_state_13 =
{
    xml_parser_state_13_transitions,
    xml_parser_state_13_transitions + 1
};

/*
14:
gt_terminal elements lt__slash_terminal name . gt_terminal ; dot_end, lt_terminal, lt__slash_terminal
shift to 15 on gt_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_14_transitions[] = 
{
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_15, NULL },
};


const sweet::parser::ParserState xml_parser_state_14 =
{
    xml_parser_state_14_transitions,
    xml_parser_state_14_transitions + 1
};

/*
15:
gt_terminal elements lt__slash_terminal name gt_terminal . ; dot_end, lt_terminal, lt__slash_terminal
reduce to end_element on dot_end ; 
reduce to end_element on lt_terminal ; 
reduce to end_element on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_15_transitions[] = 
{
    { xml_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_7 },
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_7 },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_7 },
};


const sweet::parser::ParserState xml_parser_state_15 =
{
    xml_parser_state_15_transitions,
    xml_parser_state_15_transitions + 3
};

/*
16:
elements element . ; lt_terminal, lt__slash_terminal
reduce to elements on lt_terminal ; 
reduce to elements on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_16_transitions[] = 
{
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_8 },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_8 },
};


const sweet::parser::ParserState xml_parser_state_16 =
{
    xml_parser_state_16_transitions,
    xml_parser_state_16_transitions + 2
};

/*
17:
element . ; lt_terminal, lt__slash_terminal
reduce to elements on lt_terminal ; 
reduce to elements on lt__slash_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_17_transitions[] = 
{
    { xml_SYMBOL_lt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_9 },
    { xml_SYMBOL_lt__slash_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_9 },
};


const sweet::parser::ParserState xml_parser_state_17 =
{
    xml_parser_state_17_transitions,
    xml_parser_state_17_transitions + 2
};

/*
18:
attributes attribute . ; question__gt_terminal, slash__gt_terminal, gt_terminal, name
reduce to attributes on question__gt_terminal ; 
reduce to attributes on slash__gt_terminal ; 
reduce to attributes on gt_terminal ; 
reduce to attributes on name ; 

*/

const sweet::parser::ParserTransition xml_parser_state_18_transitions[] = 
{
    { xml_SYMBOL_question__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_12 },
    { xml_SYMBOL_slash__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_12 },
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_12 },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_12 },
};


const sweet::parser::ParserState xml_parser_state_18 =
{
    xml_parser_state_18_transitions,
    xml_parser_state_18_transitions + 4
};

/*
19:
attribute . ; question__gt_terminal, slash__gt_terminal, gt_terminal, name
reduce to attributes on question__gt_terminal ; 
reduce to attributes on slash__gt_terminal ; 
reduce to attributes on gt_terminal ; 
reduce to attributes on name ; 

*/

const sweet::parser::ParserTransition xml_parser_state_19_transitions[] = 
{
    { xml_SYMBOL_question__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_13 },
    { xml_SYMBOL_slash__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_13 },
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_13 },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_13 },
};


const sweet::parser::ParserState xml_parser_state_19 =
{
    xml_parser_state_19_transitions,
    xml_parser_state_19_transitions + 4
};

/*
20:
name . eq_terminal value ; question__gt_terminal, slash__gt_terminal, gt_terminal, name
shift to 21 on eq_terminal ; 

*/

const sweet::parser::ParserTransition xml_parser_state_20_transitions[] = 
{
    { xml_SYMBOL_eq_terminal, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_21, NULL },
};


const sweet::parser::ParserState xml_parser_state_20 =
{
    xml_parser_state_20_transitions,
    xml_parser_state_20_transitions + 1
};

/*
21:
name eq_terminal . value ; question__gt_terminal, slash__gt_terminal, gt_terminal, name
shift to 22 on value ; 

*/

const sweet::parser::ParserTransition xml_parser_state_21_transitions[] = 
{
    { xml_SYMBOL_value, sweet::parser::TRANSITION_SHIFT, &xml_parser_state_22, NULL },
};


const sweet::parser::ParserState xml_parser_state_21 =
{
    xml_parser_state_21_transitions,
    xml_parser_state_21_transitions + 1
};

/*
22:
name eq_terminal value . ; question__gt_terminal, slash__gt_terminal, gt_terminal, name
reduce to attribute on question__gt_terminal ; 
reduce to attribute on slash__gt_terminal ; 
reduce to attribute on gt_terminal ; 
reduce to attribute on name ; 

*/

const sweet::parser::ParserTransition xml_parser_state_22_transitions[] = 
{
    { xml_SYMBOL_question__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_15 },
    { xml_SYMBOL_slash__gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_15 },
    { xml_SYMBOL_gt_terminal, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_15 },
    { xml_SYMBOL_name, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_15 },
};


const sweet::parser::ParserState xml_parser_state_22 =
{
    xml_parser_state_22_transitions,
    xml_parser_state_22_transitions + 4
};

/*
23:
document . ; dot_end
reduce to dot_start on dot_end ; 

*/

const sweet::parser::ParserTransition xml_parser_state_23_transitions[] = 
{
    { xml_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_0 },
};


const sweet::parser::ParserState xml_parser_state_23 =
{
    xml_parser_state_23_transitions,
    xml_parser_state_23_transitions + 1
};

/*
24:
prolog element . ; dot_end
reduce to document on dot_end ; 

*/

const sweet::parser::ParserTransition xml_parser_state_24_transitions[] = 
{
    { xml_SYMBOL_dot_end, sweet::parser::TRANSITION_REDUCE, NULL, &xml_parser_production_1 },
};


const sweet::parser::ParserState xml_parser_state_24 =
{
    xml_parser_state_24_transitions,
    xml_parser_state_24_transitions + 1
};


sweet::parser::ParserStateMachine xml_parser_state_machine = 
{
    &xml_parser_state_0,
    &xml_lexer_state_machine,
    xml_SYMBOL_dot_start,
    xml_SYMBOL_dot_end,
    xml_SYMBOL_dot_error_terminal,
    xml_parser_actions,
    xml_parser_actions + 3,
};

