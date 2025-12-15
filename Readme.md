# TimeFlow
A simple cli task scheduler made for personal use

purely in C !

## Background
My schedules are very unique from the other
common ones like list of time and tasks
I have blocks of time that adjust to whatever 
time space is available, sometimes they may strech

Although I could probably do with some
other online scheduling or todo apps, I wanted
minimal and cli

You may also like the way I schedule my time,
who knows !

## How to install
First, clone this repo
```
$ git clone https://github.com/prakash-niroula/repo
```
Then run this command to build the executable
```
$ make all
```
and... that's it !

## Types

### 1. `<Date>`
* `today` = Today
* `tomorrow` = Tomorrow
* `yesterday` = Yesterday
* `<day>` = sunday, monday, ... OR sun, mon, ...
* `<month_name>/<date>` = specific month & day<br> 
  (e.g. Dec/09)
* `<year>/<month>/<date>` = specific date
* `<Date?> + <DateDuration>` = given date plus duration.
  if date is omitted, "today" is used
* `<Date?> - <DateDuration>` = given date minus duration.
  if date is omitted, "today" is used


### 2. `<DateDuration>`
Format = `<year?>y` `<month>m` `<date>d`

Examples
`2y 2m 15d`, `1y`, `5d`

### 3. `<Time>`
* `hh:mm` = point of time
* `hh:mm - hh:mm` = start time to end time
* `now` = current time
* `now - hh:mm` = current time to given time
* `hh:mm - now` = given time to current time
* `hh:mm ...` = given time to next block start time
* `... hh:mm` = previous block end time to given time
* `... <TimeDuration>` = previous block end time + 
  duration
* `<TimeDuration> ...` = previous block end time - 
  duration
* `full` = all day
* `<Time?> + <TimeDuration>` = given time plus duration.
  if time is omitted, "now" is used.
* `<Time?> - <TimeDuration>` = given time minus duration.
  if time is omitted, "now" is used.

### 4. `<TimeDuration>`
Format = `<hours?>h` `<minutes>m`

Examples
`2h 45m`, `1h 2m`, `10m`

## Viewing Tasks

Supports 8 formats :
* `list <Date?> <styled?>` = lists the tasks in a list view.
  * `<Date?>` = optional, if omitted, today is used
  * `<styled?>` = optional, if used, differentiates
    between important, unimportant and regular tasks
    unlike default chronological order

* `grid <Date?> <styled?>` = lists the tasks in a grid view
  * `<Date?>` = optional, if omitted, today is used
  * `<styled?>` = optional, if used, differentiates
    between important, unimportant and regular tasks
    unlike default chronological order

* `current` = display the current task in detailed
  view along with subtasks

* `next` = display the next task in detailed
  view along with subtasks

* `prev` = display the previous task in detailed
  view along with subtasks

* `task <Date?> <task_number>` = provides a detailed
  view of a specific task
  * `<Date?>` = Date is optional, if omitted, today
    is used.
  * `<task_number>` = serial number of task in
    chronological order (starts from 1)

* `week` = display the weekly timeline in format :
  `<day> | (tasks?)`
  * `<day>` = 3 letter shorthand day
  * `(tasks?)` = Comma separated task names

* `month` = displays monthly timeline in format :
  `<date> (number?) <status?>`
  * `<date>` = date number
  * `(number?)` = number of tasks on that date
  * `<status?>` = optional status
    * `!` = if one of the tasks is important
    * `~` = if all of the tasks are unimportant

Examples :
```
$ timeflow list

1. *Task 0 !| 07:00 - 09:00 | 1h
2.  Task 1 !| 09:00 - 11:23 | 1h 23m
3.  Break  ~| 11:23 - 15:23 | 4h
4.  Task 4  | 16:00 - 17:00 | 1h
```

```
$ timeflow list styled

! Important
-----------
1. *Task 0 | 07:00 - 09:00 | 1h
2.  Task 4 | 16:00 - 17:00 | 1h

Regular
-----------
3. *Task 1 | 09:00 - 11:23 | 1h 23m
  (...)

~ Unimportant
-------------
4.  Break  | 11:23 - 15:23 | 4h

```

```
$ timeflow grid

07:00 - 16:00
[ *Task 0 !] - [  Task 1 ] - [  Task 2 ]
[  Task 3  ] - [  Break ~] - [  Task 4 ]

17:00 - 18:00
[  Task 7 ]

```

```
$ timeflow current

09:45 | Thu | Dec 09
======================
! Important | ( Recurring / week )
Task 0 | 09:00 - 11:23 | 2h 23m

| Task 0 very very important

- subtask 1
- subtask 2
- subtask 3
```

```
$ timeflow next

09:45 | Thu | Dec 09
======================
( Recurring / month )
Task 5 | 09:00 - 11:23 | 2h 23m

| Task 5 once a month

- subtask 1
- subtask 2
- subtask 3
```

```
$ timeflow task Dec/09 3

Thu | Dec 09
======================
( Recurring / year )
Task 8 | 10:00 - 11:23 | 1h 23m

| Task 8 once a year

- subtask 1
- subtask 2
- subtask 3
```

```
$ timeflow week

  Sun | Task1(!), Break, Task2, Task 3,
      | Task 5, Task 6, Task 7, (...)
--------------------------------------
* Mon | Task1, Break, Task2, (...)
---------------------------------------
  Tue | Break, (...)
---------------------------------------
  (...)
---------------------------------------
  Sat | Holiday
```

```
$ timeflow month

   Sun    |   Mon    |   Tue    |   Wed    |   Thu    |   Fri   |   Sat
---------------------------------------------------------------------------
          |  01 (5)  |  02      |  03      |  04      |  05     |  06
  07      |  08      | *09 (12)!|  10 (12) |  11 (9)  |  12     |  13 
(...)
```

## Adding tasks

Syntax :
```
$ timeflow add <Date?>
```
Date is optional, if omitted, today is assumed

Input :
* `Task` = Name of the task
* `Task type`
* `Desc` = Description ( can be left empty )
* `Time` = as a type of `<Time>`
* `Subtasks?` = you can add subtasks within a task
* `Recurring?` = if required, one of these options
  * `daily` = recurrs everyday
  * `weekly` = recurrs every week on the same day
  * `monthly` = recurrs every month on the same date
  * `yearly` = recurrs every year, same month & date
  * `workdays` = recurrs every work-day on given time (configurable)
  * `holidays` = recurrs every holiday on given time (configurable)

Examples :

```
$ timeflow add

Task = Task 1
Desc = Task 1 rules the world
Time = 9:00 - 11:23

(y/n) Subtasks? y
- subtask1
- subtask2
- (...)

(y/n) Recurring? y
1. daily
2. weekly
3. monthly
4. yearly

5. workdays
6. holidays
- Choice (0 for none) = 2

Task `Task1` created recurring every week...
```

```
$ timeflow add thu

Task = Break
Desc = Relax & Enjoy
Time = 11:23 - 15:23

(y/n) Subtasks? n
(y/n) Recurring? n

Task `Task 1` created for next thursday...
```

```
$ timeflow add Dec/09

Task = Special
Desc = Dec 9th is a special day
Time = full

(y/n) Subtasks? n

Task `Special` created for Dec/09...
```

```
$ timeflow add 2025/12/09

Task = Very specific date
Desc = Very very specific date
Time = full

(y/n) Subtasks? n

Task `Very specific date` created for 2025/12/09...
```

## Editing tasks

### Quickly reschedule time

Syntax :
```
$ timeflow reschedule <Date?> <TaskNumber?>
```
* `Date` is optional, if omitted, today is used
* `TaskNumber` is optional, if omitted, list is displayed
and choice is asked

Example :
```
$ timeflow reschedule today

1. Task 1 | 09:00 - 11:23 | 1h 23m
2. Break  | 11:23 - 15:23 | 4h
(...)

Task number = 1

(y/n) Reschedule Date? y
New Date = <Date>

(y/n) Reschedule Time? y
New Time = <Time>

Task `Task 1` rescheduled to new date & time...
```

Syntax :
```
$ timeflow edit <Date?>
```
Date is optional, if omitted, today is used

Input :
* `Task number` = Serial number of task in the shown list

Examples :
```
$ timeflow edit

1. Task 1 | 09:00 - 11:23 | 1h 23m
2. Break  | 11:23 - 15:23 | 4h
(...)

Task number = 1

Task name = Task 1
Desc = Task 1 is the start
Time = 09:00 - 10:00

(y/n) Subtasks? n
(y/n) Recurring? n

Task `Task 1` edited...
```

## Deleting tasks

Syntax :
```
$ timeflow delete <Date?>
```
Date is optional, if omitted, today is used

Input :
* `Task number` = Serial number of task in the shown list

Examples :
```
$ timeflow delete

1. Task 1 | 09:00 - 11:23 | 1h 23m
2. Break  | 11:23 - 15:23 | 4h
(...)

Task number = 1

Task 1 | 09:00 - 11:23 | 2h 23m

| Task 1 rules the world

- subtask 1
- subtask 2
- subtask 3

(y/n) Confirm deletion? y

Task `Task1` deleted...
```