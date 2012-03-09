   
xml {
    %whitespace "[ \t\r\n]*";
    %left '<' '>';
    %left name;
    document: prolog element;
    prolog: '<?xml' attributes '?>' | ;
    element: begin_element attributes end_element;
    begin_element: '<' name [begin_element];
    end_element: '/>' [end_element] | '>' elements '</' name '>' [end_element];
    elements: elements element | element | %precedence '<';
    name: "[A-Za-z_:][A-Za-z0-9_:\\.-]*";
    attributes: attributes attribute | attribute | %precedence name;
    attribute:  name '=' value [attribute];
    value: "[\"']:string:";
}
