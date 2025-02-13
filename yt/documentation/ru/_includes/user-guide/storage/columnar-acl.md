# Управление доступом к колонкам таблиц

В данном разделе содержится описание работы ACL на колонки, приводятся примеры использования.

## Краткое описание { #brief_desc }

ACL на колонки позволяют уточнить правила доступа к отдельным колонкам таблицы. Это позволяет хранить в одной таблице как общедоступные данные, так и приватные. При этом процессы и пользователи, которые имеют подходящий доступ, смогут читать все колонки в таблице, включая приватные, а остальные пользователи — только публичные.

## Принцип работы { #details }

Общие принципы работы системы контроля доступа в системе {{product-name}} описаны в разделе [Контроль доступа](../../../user-guide/storage/access-control.md).

Чтобы установить ACL на колонки, необходимо в ACL узлов — как самих таблиц, так и директорий — использовать ACE специального вида, в которых указан дополнительный атрибут `columns`. Подобные ACE (**колоночные**) никак не участвуют в обычных проверках прав к целым объектам. Такие ACE учитываются на стадии проверки доступа к отдельным колонкам таблиц.

Предположим, что система выполняет проверку прав доступа на чтение к таблице `T` от имени пользователя `U`, при этом данный пользователь хочет прочитать колонки из множества `Cs`. Подобные чтения происходят при выполнении команды `read-table`, а также при подаче таблицы `T` в качестве входной в операцию MapReduce или другие.

Описываемая функциональность доступна только для [схематизированных данных](../../../user-guide/storage/static-schema.md). В частности, будем считать, что `Cs` представляет собой подмножество всех колонок, явно указанных в схеме таблицы `T`.

Само множество `Cs` пользователь обычно указывает, задавая фильтр колонок, например, через [YPath](../../../user-guide/storage/ypath.md). В случае, когда никакой фильтр не указан, считается, что `Cs` — множество всех колонок, описанных в схеме таблицы. Если при этом схема таблицы нестрогая, то в самих данных могут встретиться дополнительные колонки, но для них никаких специальных проверок не производится.

Помимо наличия права `read` на всю таблицу у пользователя также должно быть право `read` на каждую из колонок множества `Сs`.

Последнее свойство проверяется следующим образом: рассмотрим имя колонки `C` из `Cs`. Построим эффективный ACL таблицы `T`. Из составляющих его ACE отберем те, которые являются колоночными и применимы к `C` (содержат `C` в атрибуте `columns`); обозначим результат через `L`.

В случае, если `L` пусто, это означает что для колонки `C` никакие особые ограничения не действуют, и проверка доступа к данной колонке считается успешной.

В противном случае, если нашлась хотя бы одна подходящая ACE, работает следующая логика. Среди построенного ранее списка `L` отберем теперь ровно те записи, которые касаются операции чтения (содержат `read` в `permissions`) и относятся к текущему пользователю `U` (`U` либо явно упомянут в `subjects`, либо принадлежит к одной из групп, упомянутых в `subjects`). Новый список обозначим через `L'`. Если в `L'` есть хотя бы одна разрешающая запись и нет ни одной запрещающей, то проверка успешна; иначе в доступе будет отказано.

## Режим совместимости { #compatibility_mode }

Существует ситуация, когда пользователи заказывают чтение всей таблицы целиком (то есть не указывают никакой колоночный фильтр) при том, что им нужна лишь малая часть колонок. Если в подобном сценарии установить на таблицу колоночный ACL, то у пользователей, не имеющих доступ к защищаемым колонкам, возникнет ошибка авторизации.

На такой случай существует специальный флаг `omit_inaccessible_columns` (значение по умолчанию `%false`), флаг можно указывать при операциях чтения. Одноименная настройка также имеется в спецификации операций.

В случае, если данная опция включена, то при невозможности получить доступ на чтение к какой-либо колонке таблицы (при наличии доступа на чтение к таблице в целом) система скрывает ее полностью от пользователя, а запрашиваемое действие при этом завершается успешно. При этом, в ответе на запрос чтения (в т. н. `output parameters`) передаётся список колонок, которые были пропущены при чтении из-за недостатка прав. В случае операции подобная информация отображается в предупреждении от планировщика.

Данная возможность активно используется в веб-интерфейсе {{product-name}}, чтобы дать возможность просмотреть данные в тех колонках, до которых у пользователя есть доступ и предупредить, что часть колонок скрыта ввиду недоступности.

## Пример { #example }

Пусть на таблице установлен ACL, в котором есть следующая ACE:

```bash
{
    action = allow;
    subjects = [username];
    permissions = [read];
    columns = [money];
}
```

Будем также считать, то на всем пути вверх до корня от данной таблицы более нет никаких колоночных ACE.

Тогда выполняется следующее:

1. Любой пользователь, у которого есть доступ `read` на данную таблицу, сможет прочитать все колонки, кроме `money`.

2. Если пользователь, отличный от `username`, попытается прочитать колонку `money`, то возникнет ошибка авторизации. Это произойдет как при явном указании имени данной колонки в фильтре, так и при попытке чтения без всякого фильтра.

3. Пользователь `username` сможет прочитать колонку `money` (при условии, что у него также есть доступ `read` на таблицу целиком).

## Замечания { #notes }

{% note warning "Внимание" %}

Установка ACE с разрешением на чтение колонки приводит к тому, что все пользователи, не указанные в ACE, автоматически лишаются права на чтение колонки. Поэтому, прежде чем ограничивать доступ к колонке, убедитесь, что подобные действия не приведут к поломке процессов, работающих с таблицей.

{% endnote %}

1. В текущей реализации функциональность распространяется лишь на чтение колонок. Поэтому не нужно указывать в атрибуте `permissions` права, отличные от `read`.
2. Если у таблицы нет схемы, либо схема пустая и слабая, то ограничить доступ к отдельным ее колонкам не получится. Не получится ограничить доступ к колонкам, которые не указаны в схеме, если схема является нестрогой, даже если при этом какие-либо ограничения для них описаны в ACL. Обязательно схематизируйте свои данные и опишите защищаемые колонки в схеме явным образом.
3. Колоночные ACE нужно указывать не только на самих таблицах, но и на директориях. При этом данные ACE наследуются стандартным образом, и их наследование, в частности, обрывается в тех узлах, где выставлено `inherit_acl = %false`).
4. Управление колоночными ACE доступно только администраторам {{product-name}}.  
