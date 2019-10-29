# coding: utf-8

from flask import Flask
from flask import request, jsonify
import mysql.connector

app = Flask(__name__)
app.config["DEBUG"] = True


# Create some test data for our catalog in the form of a list of dictionaries.
books = [
	{'id': 0,
	 'title': 'A Fire Upon the Deep',
	 'author': 'Vernor Vinge',
	 'first_sentence': 'The coldsleep itself was dreamless.',
	 'year_published': '1992'},
	{'id': 1,
	 'title': 'The Ones Who Walk Away From Omelas',
	 'author': 'Ursula K. Le Guin',
	 'first_sentence': 'With a clamor of bells that set the swallows soaring, the Festival of Summer came to the city Omelas, brig',
	 'published': '1973'},
	{'id': 2,
	 'title': 'Dhalgren',	
	 'author': 'Samuel R. Delany',
	 'first_sentence': 'to wound the autumnal city.',
	 'published': '1975'}
]

@app.route("/", methods=['GET'])
def hello():
	return '''<h1>Monitoramento do estacionamento ufabc</h1>
<p>A prototype API for distant reading oata from database.</p>'''

@app.route('/v1/resources/books/all', methods=['GET'])
def api_all():
	return jsonify(books)

@app.route('/v1/resources/books', methods=['GET'])
def api_id():
	# Check if an ID was provided as part of the URL.
	# If ID is provided, assign it to a variable.
	# If no ID is provided, display an error in the browser.
	if 'id' in request.args:
		id = int(request.args['id'])
	else:
		return "Error: No id field provided. Please specify an id."
	# Create an empty list for our results
	results = []
	# Loop through the data and match results that fit the requested ID.
	# IDs are unique, but other fields might return many results
	for book in books:
		if book['id'] == id:
			results.append(book)
	# Use the jsonify function from Flask to convert our list of
	# Python dictionaries to the JSON format.
	return jsonify(results)

@app.errorhandler(404)
def page_not_found(e):
	return "<h1>404</h1><p>The resource could not be found.</p>", 404


@app.route('/v1/resources/users', methods=['GET'])
def api_filter_user():

	query_parameters = request.args

	cod_usuario = query_parameters.get('cod_usuario')
	
	cnx = mysql.connector.connect(user='root', passwd='', database='db_estacionamento_ufabc')
	cursor = cnx.cursor()

	if not cod_usuario:
		return page_not_found(404)


	args = cursor.callproc('sp_users_list', (cod_usuario, ))
	print("Return value:", args)

	users = []
	
	for result in cursor.stored_results():
		users = users_file_json(result.fetchall())

	cursor.close()
	cnx.close()

	return jsonify(users)


def users_file_json(result):

	users = []

	for row in result:
		users.append(
        {
        'cod_usuario': row[0],
        'nome': row[1],
        'cod_categoria': row[2]
        })

	return users

if __name__ == "__main__":
	app.run()
