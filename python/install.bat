cd %cd%\venv\Scripts
call activate
cd "%~dp0"
pip install flask
pip install pyserial
echo Module has been install!
pause