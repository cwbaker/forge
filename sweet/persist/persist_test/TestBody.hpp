//
// TestBody.hpp
// Copyright (c) 2008 Charles Baker.  All rights reserved.
//

#ifndef TESTBODY_HPP_INCLUDED
#define TESTBODY_HPP_INCLUDED

template <class Writer, class Reader, class Model>
struct TestBody
{
    Model written_model_;
    Model read_model_;

    TestBody( const char* filename )
    : written_model_(),
      read_model_()
    {
        SWEET_ASSERT( filename );
        
        Writer writer;
        writer.write( filename, "model", written_model_ );
        read_model_.clear();
        Reader reader;
        reader.read( filename, "model", read_model_ );
    }
};

#endif