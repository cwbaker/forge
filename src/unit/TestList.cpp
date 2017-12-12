#include <unit/Config.h>
#include <unit/TestList.h>
#include <unit/Test.h>

#include <cassert>

namespace UnitTest {

TestList::TestList() 
    : m_head(0)
    , m_tail(0)
{
}

void TestList::Add(Test* test)
{
    if (m_tail == 0)
    {
        assert(m_head == 0);
        m_head = test;
        m_tail = test;
    }
    else
    {
        m_tail->next = test;
        m_tail = test;
    }
}

Test* TestList::GetHead() const
{
    return m_head;
}

ListAdder::ListAdder(TestList& list, Test* test)
{
    list.Add(test);
}

}
