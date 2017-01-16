from sortedcontainers import SortedSet, SortedList
# http://www.grantjenks.com/docs/sortedcontainers/index.html


class _ThresholdPossibility:
    def __init__(self, middle, difference):
        self.middle = middle
        self.difference = difference

    def __lt__(self, other):
        return self.difference < other.difference


class ThresholdFinder:
    def __init__(self, data):
        self._data = data
        self._list_of_wire_thresholds = []
        self._list_of_tunnel_thresholds = []

    @staticmethod
    def _sort_threshold_possibilities(sorted_set_of_readings):
        to_return = SortedList()
        previous_reading = None
        for reading in sorted_set_of_readings:
            if previous_reading is not None:
                to_return.add(_ThresholdPossibility((reading + previous_reading) // 2,
                                                    reading - previous_reading))
            previous_reading = reading
        return to_return

    def find_thresholds(self):
        set_of_wire_readings = SortedSet()
        set_of_tunnel_readings = SortedSet()
        for space in self._data:
            if space.tunnelReading is not None:
                set_of_tunnel_readings.add(space.tunnelReading)
            if space.wireReading is not None:
                set_of_wire_readings.add(space.wireReading)

        wire_thresholds = self._sort_threshold_possibilities(set_of_wire_readings)  # sorted ascending
        self._list_of_wire_thresholds = []
        for threshold in reversed(wire_thresholds):  # sort descending
            self._list_of_wire_thresholds.append(threshold.middle)

        tunnel_thresholds = self._sort_threshold_possibilities(set_of_tunnel_readings)  # sorted ascending
        self._list_of_tunnel_thresholds = []
        for threshold in reversed(tunnel_thresholds):  # sort descending
            self._list_of_tunnel_thresholds.append(threshold.middle)

    def get_wire_thresholds(self):
        return self._list_of_wire_thresholds

    def get_tunnel_thresholds(self):
        return self._list_of_tunnel_thresholds


class _TestGridSpaceData:
    def __init__(self, w, t):
        self.wireReading = w
        self.tunnelReading = t


def _test():
    data = [
        _TestGridSpaceData(23, 0),
        _TestGridSpaceData(78, 0),
        _TestGridSpaceData(43, 1),
        _TestGridSpaceData(2, 1),
        _TestGridSpaceData(97, 0),
        _TestGridSpaceData(51, 0),
        _TestGridSpaceData(29, 1),
        _TestGridSpaceData(51, 0),
        _TestGridSpaceData(19, 0)
    ]

    algorithm = ThresholdFinder(data)
    algorithm.find_thresholds()
    print(algorithm.get_wire_thresholds())
    # assert [64, 87, 10, 36, 47, 26, 21]
    print(algorithm.get_tunnel_thresholds())
    # assert [0]

if __name__ == "__main__":
    _test()
