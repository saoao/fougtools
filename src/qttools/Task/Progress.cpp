#include "Progress.h"

#include "BaseRunner.h"
#include "Runnable.h"

namespace Task {

Progress::Progress(Runnable *runnable)
    : m_runnable(runnable),
      m_value(0)
{
}

Progress::~Progress()
{
}

int Progress::value() const
{
    //return qttools::QAtomicTools::loadRelaxed(d->m_value);
    return m_value;
}

void Progress::setValue(int pct)
{
    //    qttools::QAtomicTools::storeRelaxed(&d->m_value, pct);
    m_value = pct;
    m_runnable->m_taskRunner->taskSignals()->emitProgress(pct);
}

const QString& Progress::step() const
{
    return m_step;
}

void Progress::setStep(const QString &title)
{
    m_step = title;
    m_runnable->m_taskRunner->taskSignals()->emitProgressStep(title);
}

void Progress::outputMessage(const QString &msg)
{
    m_runnable->m_taskRunner->taskSignals()->emitMessage(msg);
}

/*! \brief Returns this progress' custom data for the key \p key as a QVariant
 *
 * Custom progress data is useful for storing arbitrary properties in any progress.\n
 * TaskManager does not use this feature for storing data; it is provided solely for the convenience
 * of the user.
 */
QVariant Progress::data(int key) const
{
    auto it = m_dataHash.find(key);
    return it != m_dataHash.end() ? (*it).second : QVariant();
}

/*! \brief Set this progress' custom data for the key \p key to \p value
 *
 * Custom progress data is useful for storing arbitrary properties for any item. TaskManager does
 * not use this feature for storing data; it is provided solely for the convenience of the user.
 */
void Progress::setData(int key, const QVariant &value)
{
    m_dataHash.emplace(key, value);
}

bool Progress::isAbortRequested() const
{
    return m_runnable->m_taskRunner->isAbortRequested();
}

} // namespace Task