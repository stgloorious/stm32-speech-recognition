# Scripts

Here are some scripts I used during development. There are not meant as
robust or user-friendly tools, so you probably need to check the source code
to see how to use them.

## Send Files
To send files using the UART protocol, use `sendfile.py`.
Note that it will fail if the file size exceeds the micrcontroller buffer
size, which is about 16 kiB.

To record a WAV file and send that to the microcontroller, you can use
~~~
arecord --format=S16 --duration=1 --rate=16000 input.wav && ./convert-wav.py && timeout 3 ./sendfile.py output.bin
~~~
This records a sound to `input.wav`, extracts the raw waveform values using
`convert.way.py` and sends that to the microcontroller.
Note that `sendfile.py` will echo all characters it receives over UART once
the transmission completes, hence we need the timeout.

## Drawing spectrogram
`spectrograms.py` draws spectrograms calculated by TensorFlow, Numpy and
the microcontroller. For this, the microcontroller data needs to be present
in `data.txt`, which can be acquired by compiling the firmware with
`-DPRINT_SPECTROGRAM` and something like `sendfile.py output.bin > data.txt`.
The file to send in this case should be raw waveform values, which can be
created by converting a WAV file with `convert-wav.py`.
For the spectrograms calculated using Python, `input.wav` is used.

## Single FFT
To plot only one line of the spectrogram, i.e., a single FFT spectrum,
you can use `plot-fft-from-stm32.py`, which also uses the data from `data.txt`.
For comparison with the numpy implementation, you can run `plot-fft-from-numpy.py`,
which should yield approximately the same result.

## Measuring Model Performance
To evaluate the performance of the model running on the STM32 using the test
set, there is `eval_testset.py`. This automatically sends the waveforms of
the test set to the MCU and stores the result in `log.csv`, which can then be
visualized in, e.g., a confusion matrix.
