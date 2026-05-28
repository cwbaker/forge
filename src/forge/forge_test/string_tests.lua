
TestSuite {
    -- lower(), upper()
    lower_lowercases_string = function()
        CHECK_EQUAL( 'foo', lower('FOO') );
        CHECK_EQUAL( 'mixedcase', lower('MixedCase') );
    end;

    upper_uppercases_string = function()
        CHECK_EQUAL( 'FOO', upper('foo') );
        CHECK_EQUAL( 'MIXEDCASE', upper('MixedCase') );
    end;
};
