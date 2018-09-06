# cwrc_dtoc_edition
CWRC Digital Table Of Context Edition

## VOYANT TROMBONE API USAGE
To communicate with Voyant we use API endpoint(https://voyant-tools.org/trombone) trombone.

### Getting corpus id and metadata
We make a *POST* HTTP Request with query parameters below:
```php
$parameters = array(
  'palette' => 'default',
  'inkeTags' => 'false',
  'suppressTools' => 'false',
  'input' => $input,
  'tool' => 'corpus.CorpusMetadata',
  'inputFormat' => '',
  'title' => $title,
  'subTitle' => $sub_title,
);
```
1. `$input` is the path of to file(s) generated from documents datastream content.
2. `$title` and `$sub_title` are extracted from the MODS datastream of the DToC edition being created.
3. Other query parameters which are dynamically added are xpaths expressions.

Once a *POST* HTTP request is made and we get the result back from trombone we store the corpus id and metadata received
into the DToC datastream.
 
 ### Getting documents metadata
 In order to retrieve some data such as `xmlAuthorXpath`, `xmlContentXpath`, `xmlDocumentsXpath` or `xmlTitleXpath` when
 given a full Voyant URL like `http://voyant-tools.org/dtoc/?corpus=e567f073b560f0e577583f9591cf595e&docId=9a0cf19e304cd9c6b3afad7ccb2cbe9d`,
 we extract the corpus id from the given url then we make *GET* HTTP request to trombone with query parameters as follow:
 ```php
$query = array(
  'corpus' => $corpus,
  'tool' => 'corpus.DocumentsMetadata',
);
```
The request to trombone return json data with `documentsMetadata` as one of the key which in turn have a `total` key for
number of documents and a `documents` key which have listing of document information. For our use case we decode the json
data to array then we access the `parent_queryParameters` like `$request_data['documentsMetadata']['documents'][0]['parent_queryParameters']`
to parse query and extract the xml xpaths expressions.
