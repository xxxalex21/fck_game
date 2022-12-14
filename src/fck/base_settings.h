#ifndef BASESETTINGS_DGSVWIEYDMLH_H
#define BASESETTINGS_DGSVWIEYDMLH_H

#include <memory>
#include <string>

namespace fck
{

template<typename T>
class BaseSettings
{
public:
    BaseSettings() = default;
    virtual ~BaseSettings() = default;

    virtual bool loadFromFile(const std::string &file_name)
    {
        return false;
    }

    virtual bool saveToFile(const std::string &file_name)
    {
        return false;
    }

    std::string getErrorString() const
    {
        return m_error_string;
    }

    static std::shared_ptr<T> getGlobal()
    {
        return instance();
    }

    static void setGlobal(const std::shared_ptr<T> &settings)
    {
        instance() = settings;
    }

protected:
    void setErrorString(const std::string error_string)
    {
        m_error_string = error_string;
    }

private:
    static std::shared_ptr<T> &instance()
    {
        static std::shared_ptr<T> s = std::make_shared<T>();
        return s;
    }

private:
    std::string m_error_string;
};

} // namespace fck

#endif // BASESETTINGS_DGSVWIEYDMLH_H
