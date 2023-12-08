const express = require('express');
const bodyParser = require('body-parser');
const app = express();
const path = require('path');

const homeRoute = require('./routes/home');

app.use(express.static(path.join(__dirname, 'public')));
app.use(bodyParser.urlencoded({extended: false}));

app.set('view engine', 'ejs');
app.set('views','views');

app.use(homeRoute);

app.listen(3000);