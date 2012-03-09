
lua {
    %whitespace "[ \t\r\n]*";
    document: element;
    element: begin_element contents end_element;
    begin_element: name '=' '{' [begin_element];
    end_element: '}' [end_element];
    contents: contents ',' content | content;
    content: element | attribute;
    attribute: name '=' nil [nil_attribute]
             | name '=' boolean [boolean_attribute]
             | name '=' integer [integer_attribute]
             | name '=' real [real_attribute]
             | name '=' string [string_attribute]
             ;
    name: "[A-Za-z_][A-Za-z_0-9]*";
    nil: 'nil';
    boolean: 'true' | 'false';
    integer: "(\+|\-)?[0-9]+";
    real: "(\+|\-)?[0-9]+(\.[0-9]+)?((e|E)(\+|\-)?[0-9]+)?";
    string: "[\"']:string:";
}
