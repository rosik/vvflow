VVFlow. Инструкция пользователя
=======

Установка
---------

На данный момент единственным вариантом установки комплекса является сборка из исходников.
Для этого используйте команду

    $ make all

По умолчанию установка производится в домашней папке.

    $ make install

После этих манипуляций комплекс расползется по директориям

 - `~/.local/bin` - бинарники и скрипты
 - `~/.local/lib` - библиотеки
 - `~/.local/share/vvhd` - автодополнение bash

Так как комплекс устанавливается в непривычные для системы места необходимо подправить конфиги. Добавьте следующие строчки в конец файла `~/.bashrc`

    export PATH=$HOME/.local/bin:$PATH
    export LD_LIBRARY_PATH=$HOME/.local/lib/:$LD_LIBRARY_PATH
    [[ -f $HOME/.local/share/vvhd/bash_completion ]] && source $HOME/.local/share/vvhd/bash_completion

И перезагрузите его командой

    $ source ~/.bashrc

Использование
-------------

Для того, что бы что-то посчитать, первым дело нужно создать файл расчета. Этим занимается программа `vvcompose`

    usage: vvcompose COMMAND ARGS [...]

    COMMAND:
        load WHAT FILENAME
            импортирует данные из файла
            возможные варианты: hdf, body, vort, ink, ink_source
        save FILENAME
            сохраняет подготовленный файл расчета
        del  WHAT
            удаляет указанные сущности из файла
            возможные варианты: vort, ink, ink_source
            также можно удалить тело с нужным номером, например: body00
        set  VARIABLE VALUE
            устанавливает значение заданного параметра

Для команды `load` понадобится подготовленный файл с данными. Формат файла зависит от того, что мы собираемся загружать.

 - `hdf`: тот самый файл расчета, предварительно созданный при помощи `vvcompose` или сохраненный при расчете программой `vvflow`
 - `body`: просой текстовый файл в две колонки `x y`
 - `vort`: простой текстовый файл, в три колонки `x y g`, третья колонка соответствует циркуляции вихрей
 - `ink`, `ink_source`: простой текстовый файл, в три колонки `x y id`, третья колонка на результатах расчета не сказывается
