
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <string>
#include <vector>

using std::vector;
using std::string;
using namespace sweet::persist;

struct Person
{
    string name_;
    vector<Person*> friends_;

    Person()
    : name_(),
      friends_()
    {
    }

    Person( const string& name )
    : name_( name ),
      friends_()
    {
    }

    void add_friend( Person* person )
    {
        friends_.push_back( person );
    }

    template <class Archive> void persist( Archive& archive )
    {
        archive.value( "name", name_ );
        archive.refer( "friends", "friend", friends_ );
    }
};

void persist_cyclic_references_example()
{
    vector<Person> people;
    people.push_back( Person("Alfred") );
    people.push_back( Person("Ben") );
    people.push_back( Person("Camilla") );
    people.push_back( Person("Denise") );

    Person& alfred = people[0];
    Person& ben = people[1];
    Person& camilla = people[2];
    Person& denise = people[3];

    alfred.add_friend( &ben );
    alfred.add_friend( &camilla );
    ben.add_friend( &alfred );
    ben.add_friend( &denise );
    camilla.add_friend( &alfred );
    camilla.add_friend( &ben );
    camilla.add_friend( &denise );
    denise.add_friend( &camilla );
    
    XmlWriter xml_writer;
    xml_writer.write( "persist_cyclic_references_example.xml", "people", "person", people );

    people.clear();
    XmlReader xml_reader;
    xml_reader.read( "persist_cyclic_references_example.xml", "people", "person", people );
}
