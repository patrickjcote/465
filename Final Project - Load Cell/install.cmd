@echo off

if exist "C:\Python27\" (
	echo Python 2.7 already installed
) else (
echo Installing Python 2.7
Setup\python-2.7.12.msi
)

if exist "C:\Python27\Lib\site-packages\pyserial-2.7-py2.7.egg-info" (
	echo PySerial already installed
) else (
echo Installing PySerial
Setup\pyserial-2.7.win32.exe
)

if exist "C:\Python27\Lib\site-packages\numpy" (
	echo NumPy already installed
) else (
echo Copying NumPy to Python Directory...
copy "Setup\numpy-1.11.3+mkl-cp27-cp27m-win32.whl" "C:\Python27\Scripts\numpy-1.11.3+mkl-cp27-cp27m-win32.whl"
echo Installing NumPy...
C:\Python27\Scripts\pip install numpy-1.11.3+mkl-cp27-cp27m-win32.whl
)
if exist "C:\Python27\Lib\site-packages\matplotlib" (
	echo Matplotlib already installed
) else (
echo Copying Matplotlib to Python Directory...
copy "Setup\matplotlib-2.0.0-cp27-cp27m-win32.whl" "C:\Python27\Scripts\matplotlib-2.0.0-cp27-cp27m-win32.whl"
echo Installating Matplotlib...
C:\Python27\Scripts\pip install matplotlib-2.0.0-cp27-cp27m-win32.whl
)
echo Installation Complete
pause
