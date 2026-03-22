"""Integration tests for the command-only Ref deployment."""


def test_send_command(fprime_test_api):
    """Verify the command dispatcher accepts commands."""
    fprime_test_api.send_and_assert_command("CdhCore.cmdDisp.CMD_NO_OP", max_delay=0.1)
    assert fprime_test_api.get_command_test_history().size() == 1
    fprime_test_api.send_and_assert_command("CdhCore.cmdDisp.CMD_NO_OP", max_delay=0.1)
    assert fprime_test_api.get_command_test_history().size() == 2


def test_send_command_args(fprime_test_api):
    """Verify commands with arguments are accepted."""
    for count, value in enumerate(["Test String 1", "Some other string"], 1):
        fprime_test_api.send_and_assert_command(
            "CdhCore.cmdDisp.CMD_NO_OP_STRING",
            [value],
            max_delay=0.1,
        )
        assert fprime_test_api.get_command_test_history().size() == count
