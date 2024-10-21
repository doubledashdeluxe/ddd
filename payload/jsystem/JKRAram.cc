#include "JKRAram.hh"

JKRAramHeap *JKRAram::GetHeap() {
    return s_aramObject->m_heap;
}
