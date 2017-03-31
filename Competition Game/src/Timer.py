from time import time
from time import sleep  # only for tests

START_VERIFICATION_FAIL_MESSAGE = "Timer not started - call start()"
PAUSE_VERIFICATION_FAIL_MESSAGE = "Timer not paused"


class Timer:
    """
    basically a stopwatch
    """
    def __init__(self):
        self._start_time = None  # value returned from time()
        self._paused_time = None  # amount

    def clear(self):
        """
        return timer to non-started state
        """
        self.__init__()

    def start(self):
        """
        starts the timer counting from 0
        """
        self._start_time = time()

    def get_elapsed_time(self):
        """
        :return: float - the number of seconds the timer has been running
        """
        self._verify_started()
        if self._paused_time is not None:
            return self._paused_time
        return time() - self._start_time

    def pause(self):
        """
        pause the timer so the time will not increment
        """
        # self._verify_started()  # get_elapsed_time() does this
        self._paused_time = self.get_elapsed_time()

    def unpause(self):
        """
        un-pause the timer - it will increment from where it was paused
        """
        self._verify_paused()
        self._start_time = time() - self._paused_time
        self._paused_time = None

    def _verify_started(self):
        if self._start_time is None:
            raise Exception(START_VERIFICATION_FAIL_MESSAGE)

    def _verify_paused(self):
        self._verify_started()
        if self._paused_time is None:
            raise Exception(PAUSE_VERIFICATION_FAIL_MESSAGE)


def unit_tests():
    print("running tests... (should take about 6 seconds)")
    a = Timer()

    start_verification_passed = False
    try:
        a.get_elapsed_time()
    except Exception as e:
        assert e.message == START_VERIFICATION_FAIL_MESSAGE
        start_verification_passed = True
    assert start_verification_passed

    a.start()
    assert a.get_elapsed_time() < .01
    sleep(1)
    assert .99 < a.get_elapsed_time() < 1.01

    pause_verification_passed = False
    try:
        a.unpause()
    except Exception as e:
        assert e.message == PAUSE_VERIFICATION_FAIL_MESSAGE
        pause_verification_passed = True
    assert pause_verification_passed

    sleep(1)
    assert 1.99 < a.get_elapsed_time() < 2.01
    a.pause()
    sleep(2)
    assert 1.99 < a.get_elapsed_time() < 2.01
    a.unpause()
    sleep(2)
    assert 3.99 < a.get_elapsed_time() < 4.01

    a.clear()

    a.start()
    assert a.get_elapsed_time() < .01

    print("all tests passed")


# TODO: run this on the pi
def main():
    unit_tests()

if __name__ == "__main__":
    main()
