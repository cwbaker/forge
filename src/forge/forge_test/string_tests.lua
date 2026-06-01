
SUITE 'string-tests';

TEST('lower_converts_to_lowercase', function()
    CHECK_EQUAL( 'foo', lower('FOO') );
    CHECK_EQUAL( 'mixedcase', lower('MixedCase') );
end);

TEST('upper_converts_to_uppercase', function()
    CHECK_EQUAL( 'FOO', upper('foo') );
    CHECK_EQUAL( 'MIXEDCASE', upper('MixedCase') );
end);
