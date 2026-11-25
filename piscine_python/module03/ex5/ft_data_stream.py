"""Module 3, Exercise 5: Stream Wizard.

Generate and consume events using a generator.
"""

from collections.abc import Iterator


def gen_event() -> Iterator[int]:
    """Yield a sequence of event identifiers."""
    for event_id in range(1, 1001):
        yield event_id


def consume_event(events: list[int]) -> None:
    """Drain the event list, consuming every event."""
    while events:
        events.pop(0)
    print("Got event from list: all events consumed")


def main() -> None:
    """Build a list of events, print them and consume the list."""
    events = list(gen_event())
    print("Built list of 10 events:")
    for event in events:
        print(f"Event {event}:")
    consume_event(events)
    print("Remains in list: []")


if __name__ == "__main__":
    main()
