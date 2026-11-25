<?php
// ============================================================================ //
//  ft_services — phpMyAdmin configuration (credentials from env/Secret)     //
// ============================================================================ //

// Authentication type: cookie
$cfg['Servers'][1]['auth_type'] = 'cookie';

// Server host — the MySQL service name (injected via env)
$cfg['Servers'][1]['host'] = getenv('PMA_HOST') ?: 'mysql';
$cfg['Servers'][1]['port'] = getenv('PMA_PORT') ?: '3306';

// Socket (optional)
if ($socket = getenv('PMA_SOCKET')) {
    $cfg['Servers'][1]['socket'] = $socket;
}

// Default user (overridable via env) — password is entered at login
$cfg['Servers'][1]['user'] = getenv('PMA_USER') ?: 'root';
$cfg['Servers'][1]['onlyuser'] = false;

// Blowfish secret for cookie auth (should be a strong random value)
$cfg['blowfish_secret'] = getenv('PMA_SECRET') ?: 'ft_services_blowfish_secret';

// Navigation tree / display settings
$cfg['Servers'][1]['pma__navigation__mode'] = 'horizontal';
$cfg['Servers'][1]['pma__navigation__hint'] = false;

// Allow root login with empty password? No — secure by default
$cfg['Servers'][1]['nobody'] = '-';

// Upload / import limits
$cfg['UploadDir'] = '/tmp';
$cfg['SaveDir'] = '/tmp';
