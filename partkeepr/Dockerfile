FROM richarvey/nginx-php-fpm:php5
MAINTAINER Timo Zimmermann <zimmermann.emb@googlemail.com>

#ENV DEBIAN_FRONTEND noninteractive

RUN apk update && \
    apk add --no-cache \
    php5-cli \
    php5-phar \
    php5-ldap \
    php5-bcmath

RUN curl -sS https://getcomposer.org/installer | php -- --install-dir=/usr/local/bin --filename=composer

ADD nginx.conf /etc/nginx/sites-enabled/default.conf
#RUN cd /var/www/html/ && cp app/config/parameters.php.dist app/config/parameters.php && composer install
