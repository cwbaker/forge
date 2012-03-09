
#include <sweet/persist/persist.hpp>
#include <sweet/persist/vector.hpp>
#include <string>
#include <vector>

using std::vector;
using std::string;
using namespace sweet::persist;

enum Category
{
    CATEGORY_NONE = 0x00,
    CATEGORY_SOCIAL = 0x01,
    CATEGORY_WORK = 0x02,
    CATEGORY_CUSTOMER = 0x04
};

enum Status
{
    STATUS_UNKNOWN,
    STATUS_ACTIVE,
    STATUS_INACTIVE
};

struct Contact
{
    string email_;
    int categories_;
    Status status_;

    public:
        Contact()
        : email_(),
          categories_(),
          status_()
        {
        }

        Contact( const string& email, int categories, Status status )
        : email_( email ),
          categories_( categories ),
          status_( status )
        {
        }

        template <class Archive> void persist( Archive& archive )
        {
            static const MaskFilter::Conversion CATEGORY[] =
            {
                { CATEGORY_NONE, "None" },
                { CATEGORY_SOCIAL, "Social" },
                { CATEGORY_WORK, "Work" },
                { CATEGORY_CUSTOMER, "Customer" },
                { 0, 0 }
            };

            static const EnumFilter::Conversion STATUS[] =
            {
                { STATUS_UNKNOWN, "Unknown" },
                { STATUS_ACTIVE, "Active" },
                { STATUS_INACTIVE, "Inactive" },
                { 0, 0 }
            };

            archive.value( "email", email_ );
            archive.value( "categories", categories_, mask_filter(CATEGORY) );
            archive.value( "status", status_, enum_filter(STATUS) );
        }
};

void persist_filters_example()
{
    vector<Contact> contacts;
    contacts.push_back( Contact("alfred@example.com", CATEGORY_NONE, STATUS_ACTIVE) );
    contacts.push_back( Contact("ben@example.com", CATEGORY_SOCIAL, STATUS_INACTIVE) );
    contacts.push_back( Contact("camilla@example.com", CATEGORY_SOCIAL | CATEGORY_WORK, STATUS_INACTIVE) );
    contacts.push_back( Contact("denise@example.com", CATEGORY_WORK | CATEGORY_CUSTOMER, STATUS_ACTIVE) );

    JsonWriter json_writer;
    json_writer.write( "persist_filters_example.json", "people", "person", contacts );

    contacts.clear();
    JsonReader json_reader;
    json_reader.read( "persist_filters_example.json", "people", "person", contacts );
}
