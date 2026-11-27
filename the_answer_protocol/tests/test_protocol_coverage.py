import protocol


def test_error_message_without_symbol_preserves_optional_field_defaults():
    message = protocol.parse_message("ERR 499")
    assert message.kind == "err"
    assert message.code == 499
    assert message.symbol == ""
    assert message.to_dict()["data"] == []
