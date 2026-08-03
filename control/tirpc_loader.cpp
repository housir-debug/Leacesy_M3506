// tirpcloader.cpp
#include "tirpc_loader.h"
#include <arpa/inet.h>

Q_LOGGING_CATEGORY(libtripc, "LIBTRIPC:")

bool TirpcDynamicLoader::load(){
    if (!m_loaded) {
        const char* lib_names[] = {"libtirpc.so.3","libtirpc.so.1","libtirpc.so",nullptr};

        for (int i = 0; lib_names[i] != nullptr; i++) {
            m_library.setFileName(lib_names[i]);
            if (m_library.load()) {
                m_pmap_set = (pmap_set_t)m_library.resolve("pmap_set");
                m_pmap_unset = (pmap_unset_t)m_library.resolve("pmap_unset");
                m_pmap_getport = (pmap_getport_t)m_library.resolve("pmap_getport");

                if (m_pmap_set && m_pmap_unset && m_pmap_getport) {
                    m_loaded = true;
                    return true;
                }

                m_library.unload();
            }
        }

        qCWarning(libtripc)<<"[load]:Dynamic load failed!:";
        return false;
    }

    return true;
}

TirpcDynamicLoader& TirpcDynamicLoader::instance(){
    static TirpcDynamicLoader loader;
    return loader;
}

bool TirpcDynamicLoader::smart_pmap_set(quint32 program, quint32 version, int protocol, quint16 port)
{
    if (m_loaded){
        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // check local rpcbind
        quint16 currentPort = m_pmap_getport(&addr, program, version, protocol);

        if (currentPort == port) {return true;}
        if (currentPort != 0) {m_pmap_unset(program, version);}
        if (m_pmap_set(program, version, protocol, port)) {return true;}

        qCDebug(libtripc) << "[smart_pmap_set]: Failed to register Program:" << program;
        return false;
    }

    qCWarning(libtripc)<<"[smart_pmap_set]:library not load!";
    return false;
}
