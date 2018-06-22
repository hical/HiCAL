# Judgments model

The judgments model contains information associated with a document judgment instance that we are storing in the database.


## Fields
The model currently have the following fields.

| Field                | Description                              | Type         |
| -------------------- | ---------------------------------------- | ------------ |
| `user`               | The user making the document judgment    | `ForeignKey` |
| `doc_id`             | The id of the judged document            | `CharField`  |
| `doc_title`          | The title of the document                | `CharField`  |
| `doc_CAL_snippet`    | The snippet selected by the CAL model for this document. Available only through the CAL interface. | `TextField`  |
| `doc_search_snippet` | The snippet the search model have generated. Available only when search is enabled. | `TextField`  |
| `query`              | The query used from the search interface that retrieved the document. | `TextField`  |
| TODO                 |                                          |              |
|                      |                                          |              |
|                      |                                          |              |

