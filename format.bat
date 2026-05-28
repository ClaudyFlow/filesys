@echo off

for /r "%~dp0inc" %%f in (*.h *.hh *.hpp *.hxx) do astyle --style=java --suffix=none --lineend=linux "%%f"
for /r "%~dp0src" %%f in (*.c *.cc *.cpp *.cxx) do astyle --style=java --suffix=none --lineend=linux "%%f"
