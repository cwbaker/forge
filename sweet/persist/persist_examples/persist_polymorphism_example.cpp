
#include <string>
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>

using namespace std;
using namespace sweet::persist;

struct Object
{
    string name_;
    
    Object()
    : name_()
    {
    }
    
    Object( const string& name )
    : name_(  name )
    {
    }
    
    virtual ~Object()
    {
    }
    
    template <class Archive> void persist( Archive& archive )
    {
        archive.value( "name", name_ );
    }
};

struct LeftObject : public Object
{
    int left_value_;

    LeftObject()
    : Object(),
      left_value_( 0 )
    {
    }

    LeftObject( const string& name, int left_value )
    : Object( name ),
      left_value_( left_value )
    {
    }

    template <class Archive> void persist( Archive& archive )
    {
        Object::persist( archive );
        archive.value( "left_value", left_value_ );
    }
};

struct RightObject : public Object
{
    float right_value_;
    
    RightObject()
    : Object(),
      right_value_( 0.0f )
    {
    }
    
    RightObject( const string& name, float right_value )
    : Object( name ),
      right_value_( right_value )
    {
    }

    template <class Archive> void persist( Archive& archive )
    {
        Object::persist( archive );
        archive.value( "right_value", right_value_ );
    }
};

struct Polymorphism
{
    vector<Object*> objects_;
    
    public:
        Polymorphism()
        : objects_()
        {
        }
        
        ~Polymorphism()
        {
            while ( !objects_.empty() )
            {
                delete objects_.back();
                objects_.pop_back();
            }
        }
        
        void create()
        {
            objects_.push_back( new Object("object_0") );
            objects_.push_back( new LeftObject("left_0", 0) );
            objects_.push_back( new RightObject("right_0", 0.0f) );
            objects_.push_back( new LeftObject("left_1", 1) );
            objects_.push_back( new LeftObject("left_2", 2) );
            objects_.push_back( new RightObject("right_1", 1.0f) );
            objects_.push_back( new RightObject("right_2", 2.0f) );
            objects_.push_back( new LeftObject("left_3", 3) );
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Polymorphism", 1, *this );
            archive.value( "objects", "object", objects_ );
        }
};

template <class Archive> void declare( Archive& archive )
{
    archive.declare<Object>( "Object", PERSIST_POLYMORPHIC );
    archive.declare<LeftObject>( "LeftObject", PERSIST_POLYMORPHIC );
    archive.declare<RightObject>( "RightObject", PERSIST_POLYMORPHIC );
}

void persist_polymorphism_example()
{
    Polymorphism polymorphism;
    polymorphism.create();
    JsonWriter json_writer;
    declare( json_writer );
    json_writer.write( "persist_polymorphism_example.json", "polymorphism", polymorphism );

    Polymorphism other_polymorphism;
    JsonReader json_reader;
    declare( json_reader );
    json_reader.read( "persist_polymorphism_example.json", "polymorphism", other_polymorphism );
}
