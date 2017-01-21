class Display:
    def __init__(self):
        self.matrix = ["E" for i in range(64)]

    def set8x8(self, index, value):
        self.matrix[index] = value
