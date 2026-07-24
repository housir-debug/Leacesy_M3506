#pragma once
#include <QLibrary>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(libtripc)

typedef int bool_t;

class TirpcDynamicLoader
{
public:
    bool load();
    static TirpcDynamicLoader& instance();
    bool smart_pmap_set(quint32 program, quint32 version, int protocol, quint16 port);

private:
    TirpcDynamicLoader() = default;
    ~TirpcDynamicLoader() = default;

    typedef bool_t (*pmap_unset_t)(unsigned long, unsigned long);
    typedef bool_t (*pmap_set_t)(unsigned long, unsigned long, unsigned int, unsigned short);
    typedef unsigned short (*pmap_getport_t)(struct sockaddr_in *, unsigned long, unsigned long, unsigned int);

    QLibrary m_library;
    bool m_loaded{false};
    pmap_set_t m_pmap_set = nullptr;
    pmap_unset_t m_pmap_unset = nullptr;
    pmap_getport_t m_pmap_getport = nullptr;
};
