#include "id_generator.h"

namespace cflow::utils {

IDGenerator::IDGenerator() : m_id(0) {}

long IDGenerator::generate()
{
    std::unique_lock<std::mutex> lk(m_idLock);
    m_id++;
    return m_id;
}

} // namespace cflow::utils