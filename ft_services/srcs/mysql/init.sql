-- ============================================================================ #
--  ft_services — MySQL/MariaDB init script                                  #
--  Creates the WordPress database + user (credentials from env/Secret).    #
-- ============================================================================ #
CREATE DATABASE IF NOT EXISTS `wordpress`
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_general_ci;

-- Application user for WordPress (password supplied via env/Secret)
-- Note: the actual password is set at runtime by the entrypoint to keep it
-- out of this file. Here we only grant privileges on the placeholder user.
GRANT ALL PRIVILEGES ON `wordpress`.* TO 'wordpress'@'%';

FLUSH PRIVILEGES;
