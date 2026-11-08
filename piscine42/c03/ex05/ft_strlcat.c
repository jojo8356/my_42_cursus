/* Comme strlcat : concatene src dans dest sans depasser size octets
   (terminaison '\0' incluse). Renvoie strlen(src) + min(size, strlen(dest)). */
unsigned int	ft_strlcat(char *dest, char *src, unsigned int size)
{
	unsigned int	dlen;
	unsigned int	slen;
	unsigned int	i;

	dlen = 0;
	while (dest[dlen] && dlen < size)
		dlen++;
	slen = 0;
	while (src[slen])
		slen++;
	if (dlen == size)
		return (size + slen);
	i = 0;
	while (src[i] && dlen + i < size - 1)
	{
		dest[dlen + i] = src[i];
		i++;
	}
	dest[dlen + i] = '\0';
	return (dlen + slen);
}
