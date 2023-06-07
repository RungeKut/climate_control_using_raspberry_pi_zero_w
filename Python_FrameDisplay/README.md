# Waveshare E-Ink, 7.5 inch, Version 1, 640×384

[Проблема с контрастом](https://stackoverflow.com/questions/70318231/e-ink-display-shows-weird-behaviour-with-horizontal-lines-no-contrast)

#Если такой процесс уже есть то убиваем и запускаем этот
def get_pids(name):
    return list(map(int,subprocess.check_output(["pgrep", "-f", name]).split()))
pid = os.getpid()
print(pid)
processes = get_pids("mary_test.py")
print(processes)
for val in processes:
    if (val != pid):
        p = subprocess.Popen(['python3', 'mary_test.py'], shell=True, preexec_fn=val)
        p.terminate()

ps ax | grep 'python3 mary_test.py'

sudo python3 -m pip install pymysql