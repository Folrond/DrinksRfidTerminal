/*
* "Drinks" RFID Terminal
* Buy sodas with your company badge!
*
* Benoit Blanchon 2014 - MIT License
* https://github.com/bblanchon/DrinksRfidTerminal
*/

#include <Arduino.h>
#include <JsonParser.h>

#include "HashBuilder.h"
#include "HttpSyncTransaction.h"

bool HttpSyncTransaction::send()
{
	buffer[0] = 0;

	return http.perform("GET /drinks/api/sync", buffer, sizeof(buffer));
}

bool HttpSyncTransaction::parse()
{
	JsonParser<13> parser;

	JsonHashTable root = parser.parseHashTable(buffer);
	if (!root.success()) return false;

	header = root.getString("Header");
	if (header == NULL) return false;

	JsonArray productsArray = root.getArray("Products");
	if (!productsArray.success()) return false;

	int count = productsArray.getLength();
	for (int i = 0; i < count; i++)
	{
		products[i] = productsArray.getString(i);
	}
	products[count] = NULL;

	time = root.getString("Time");
	if (time == NULL) return false;

	hash = root.getString("Hash");
	if (hash == NULL) return false;

	return true;
}

bool HttpSyncTransaction::validate()
{
	HashBuilder hashBuilder;

	hashBuilder.print(header);

	for (int i = 0; products[i] != NULL; i++)
		hashBuilder.print(products[i]);

	hashBuilder.print(time);

	return strcasecmp(hash, hashBuilder.getHash()) == 0;
}

void HttpSyncTransaction::getCatalog(Catalog& catalog)
{
	catalog.setHeader(header);

	int i;
	for (i = 0; i < 4; i++)
	{
		if (products[i] == NULL) break;
		catalog.setProduct(i, products[i]);
	}

	catalog.setProductCount(i);
}
