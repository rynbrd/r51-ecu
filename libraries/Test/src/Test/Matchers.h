#ifndef _R51_TEST_MATCHERS_H_
#define _R51_TEST_MATCHERS_H_

#define assertStringsEqual(left, right) do { \
    if (strcmp((char*)left, (char*)right) != 0) { \
        SERIAL_PORT_MONITOR.print("\""); \
        SERIAL_PORT_MONITOR.print((char*)left); \
        SERIAL_PORT_MONITOR.print("\" != \""); \
        SERIAL_PORT_MONITOR.println((char*)right); \
        SERIAL_PORT_MONITOR.print("\""); \
        assertTrue(false); \
    } \
} while (false)

#define assertPrintablesEqual(left, right) do { \
    if (left != right) { \
        SERIAL_PORT_MONITOR.print(left); \
        SERIAL_PORT_MONITOR.print(" != "); \
        SERIAL_PORT_MONITOR.println(right); \
        assertTrue(false); \
    } \
} while (false)

#define assertSize(container, value) \
    assertEqual((int)container.size(), (int)value)

#define assertIsEvent(message, event_) \
    assertEqual(message.type(), R51::Message::EVENT); \
    assertPrintablesEqual(message.event(), event_);

#define assertIsCANFrame(message, frame) \
    assertEqual(message.type(), R51::Message::CAN_FRAME);\
    assertPrintablesEqual(message.can_frame(), frame);

#endif  // _R51_TEST_MATCHERS_H_
