#include "NameEntryHelper.hh"

bool NameEntryHelper::wasSelected() const {
    return m_state == State::Selected;
}

bool NameEntryHelper::wasCanceled() const {
    return m_state == State::Canceled;
}

void NameEntryHelper::stopEdit() {
    if (m_state != State::Selected && m_state != State::Canceled) {
        m_drums[m_index]->selectOut();
    }
    m_state = State::Inactive;
}
