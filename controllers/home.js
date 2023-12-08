const getHome = (req, res, next) => {
    res.render('home');
};

const postHome = (req, res, next) => {
    res.render('home');
}

module.exports = {
    getHome: getHome,
    postHome: postHome
}