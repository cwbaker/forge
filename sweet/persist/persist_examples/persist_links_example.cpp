
#include <sweet/persist/persist.hpp>
#include <map>

using std::map;
using std::string;
using namespace sweet::persist;

class TextureCache;

class Texture
{
    string filename_;
    TextureCache* manager_;

    public:
        Texture( const string& filename, TextureCache* manager )
        : filename_( filename ),
          manager_( manager )
        {
        }
        
        const string& filename() const 
        {
            return filename_;
        }
};

class TextureCache
{
    map<string, Texture*> textures_;

    public:
        TextureCache()
        : textures_()
        {
        }

        Texture* texture( const string& filename )
        {
            Texture* texture = NULL;
            map<string, Texture*>::iterator i = textures_.find( filename );
            if ( i != textures_.end() )
            {
                texture = i->second;
            }
            else
            {
                texture = new Texture( filename, this );
                textures_.insert( make_pair(filename, texture) );
            }
            return texture;
        }

        void release( Texture* texture )
        {
            if ( texture )
            {
                SWEET_ASSERT( textures_.find(texture->filename()) != textures_.end() );
                map<string, Texture*>::iterator i = textures_.find( texture->filename() );
                textures_.erase( i );
                delete texture;
            }
        }
};

template <class Archive> 
void save( Archive& archive, int mode, const char* name, Texture* texture )
{
    ObjectGuard<Archive> guard( archive, name, NULL, mode, texture ? 1 : 0 );
    if ( texture )
    {
        string filename = texture->filename();
        archive.value( "filename", filename, path_filter(archive.get_path()) );
    }
}

template <class Archive> 
void load( Archive& archive, int mode, const char* name, Texture*& texture )
{
    SWEET_ASSERT( texture == NULL );

    ObjectGuard<Archive> guard( archive, name, NULL, mode );
    if ( archive.is_object() )
    {
        string filename;
        archive.value( "filename", filename, path_filter(archive.get_path()) );

        TextureCache* texture_manager = reinterpret_cast<TextureCache*>( archive.get_context(SWEET_STATIC_TYPEID(TextureCache)) );
        SWEET_ASSERT( texture_manager );
        texture = texture_manager->texture( filename );
    }
}

template <class Archive>
void resolve( Archive& archive, int mode, Texture*& texture )
{
    ObjectGuard<Archive> guard( archive, NULL, NULL, mode );
}

class Model
{
    TextureCache texture_cache_;
    Texture* texture_;
    Texture* other_texture_;

    public:
        Model()
        : texture_cache_(),
          texture_( NULL ),
          other_texture_( NULL )
        {
        }
        
        ~Model()
        {
            texture_cache_.release( other_texture_ );
            other_texture_ = NULL;            
            texture_cache_.release( texture_ );
            texture_ = NULL;
        }

        void create()
        {
            string cwd = narrow(sweet::path::current_working_directory().string());
            texture_ = texture_cache_.texture( cwd + "/textures/texture.tga" );
            other_texture_ = texture_cache_.texture( cwd + "/textures/other_texture.tga" );
        }

        template <class Archive> void enter( Archive& archive )
        {
            archive.set_context( SWEET_STATIC_TYPEID(TextureCache), &texture_cache_ );
        }

        template <class Archive> void persist( Archive& archive )
        {
            archive.enter( "Textures", 1, *this );
            archive.value( "texture", texture_ );
            archive.value( "other_texture", other_texture_ );
        }
};

void persist_links_example()
{
    Model model;
    model.create();
    JsonWriter json_writer;
    model.enter( json_writer );
    json_writer.write( "persist_links_example.json", "model", model );

    Model other_model;
    JsonReader json_reader;
    other_model.enter( json_reader );
    json_reader.read( "persist_links_example.json", "model", other_model );
}
