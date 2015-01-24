#include <QtTest/QtTest>

#include "iostream"

#include "ChatMessagesTest.h"
#include "ReliableTextReceiverTest.h"

template<class Test>
static int runTest()
{
    Test test;
    return QTest::qExec(&test);
}

/**
 * Each test class should be registered in this function.
 */
int runTests()
{
    int result = 0;

    result += runTest<ChatMessageTest>();
    result += runTest<ReliableTextReceiverTest>();

    if (result > 0) {
        std::cout << "\nATTENTION: " << result << " test(s) failed.\n\n";
    } else {
        std::cout << "\nAll tests passed.\n\n";
    }
    return result;
}
