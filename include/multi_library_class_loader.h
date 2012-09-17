#ifndef PLUGINS_MULTI_LIBRARY_CLASS_LOADER_H_DEFINED
#define PLUGINS_MULTI_LIBRARY_CLASS_LOADER_H_DEFINED

#include "class_loader.h"
#include <boost/thread.hpp>

namespace plugins
{

typedef std::string LibraryPath;
typedef std::map<LibraryPath, plugins::ClassLoader*> LibraryToClassLoaderMap;
typedef std::vector<ClassLoader*> ClassLoaderVector;

class MultiLibraryClassLoader
{
  public:
    MultiLibraryClassLoader(bool enable_ondemand_loadunload);
    virtual ~MultiLibraryClassLoader();

    /**
     *
     */
    template <class Base>
    bool isClassAvailable(const std::string& class_name)
    {
      std::vector<std::string> available_classes = getAvailableClasses<Base>();
      return(available_classes.end() != std::find(available_classes.begin(), available_classes.end(), class_name));
    }

    /**
     *
     */
    bool isLibraryAvailable(const std::string& library_name);

    /**
     *
     */
    template <class Base>
    std::vector<std::string> getAvailableClasses()
    {
      std::vector<std::string> available_classes;
      ClassLoaderVector loaders = getAllAvailableClassLoaders();
      for(unsigned int c = 0; c < loaders.size(); c++)
      {
        ClassLoader* current = loaders.at(c);
        std::vector<std::string> loader_classes = current->getAvailableClasses<Base>();
        available_classes.insert(available_classes.end(), loader_classes.begin(), loader_classes.end());
      }
      return(available_classes);
    }

    /**
     *
     */
    std::vector<std::string> getRegisteredLibraries();

    /**
     *
     */
    template <class Base>
    boost::shared_ptr<Base> createInstance(const std::string& class_name, const std::string& library_path)
    {
      return(getClassLoaderForLibrary(library_path)->createInstance<Base>(class_name));
    }

    /**
     *
     */
    void loadLibrary(const std::string& library_path);

    /**
     *
     */
    void unloadLibrary(const std::string& library_path);


  private:
    bool isOnDemandLoadUnloadEnabled(){return(enable_ondemand_loadunload_);}
    ClassLoader* getClassLoaderForLibrary(const std::string& library_path);
    ClassLoaderVector getAllAvailableClassLoaders();
    void shutdownAllClassLoaders();

  private:
    bool enable_ondemand_loadunload_;
    LibraryToClassLoaderMap active_class_loaders_;
    boost::mutex loader_mutex_;
};


}
#endif