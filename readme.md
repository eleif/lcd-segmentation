# Enumerating LCD segments with OpenCV (reverse-engineering)

## Contents
- The [Python 3](https://www.python.org/downloads/) script [lcd-segmentation.py](lcd-segmentation.py) processes a video (e.g [demo.mp4](demo.mp4)) using OpenCV (requires NumPy, install both via pip) to automatically enumerate all 125 segments of an undocumented LCD, in order to ease reverse-engineering efforts (mapping digits to write and read)
- The "Arduino" code [lcd-segmentation/lcd-segmentation.ino](lcd-segmentation/lcd-segmentation.ino) was used to control the display in the demo

## Info
- writeup with demo video at https://eleif.net/lcd-segmentation.html
- see sources

## License

GNU GPLv3 (see [LICENSE](LICENSE))
