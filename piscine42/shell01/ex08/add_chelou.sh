#!/bin/sh
# Additionne FT_NBR1 (base \'\"?!) et FT_NBR2 (base mrdoc),
# puis affiche la somme ecrite dans la base "gtaio luSnemf" (base 13).
# L'arithmetique (grands entiers, base 13) se fait entierement en awk :
# chaque nombre est converti dans un tableau base 13, puis on additionne.
printf '%s\n%s\n' "$FT_NBR1" "$FT_NBR2" | awk '
BEGIN {
	b1 = "\\\047\"?!"
	b2 = "mrdoc"
	b3 = "gtaio luSnemf"
	A[0] = 0
	B[0] = 0
}
{
	base = b1
	if (NR == 2)
		base = b2
	radix = length(base)
	i = 1
	while (i <= length($0)) {
		d = index(base, substr($0, i, 1)) - 1
		if (d < 0) {
			err = 1
			exit
		}
		if (NR == 1)
			muladd(A, radix, d)
		else
			muladd(B, radix, d)
		i++
	}
}
END {
	if (err) {
		print "Error"
		exit
	}
	addnum(A, B)
	if (A[0] == 0)
		print substr(b3, 1, 1)
	else {
		out = ""
		i = A[0]
		while (i >= 1) {
			out = out substr(b3, A[i] + 1, 1)
			i--
		}
		print out
	}
}
# multiplie le grand entier arr (base 13, arr[0] = taille) par radix
# et ajoute le petit chiffre digit
function muladd(arr, radix, digit, carry, i, t) {
	carry = digit
	i = 1
	while (i <= arr[0]) {
		t = arr[i] * radix + carry
		arr[i] = t % 13
		carry = int(t / 13)
		i++
	}
	while (carry > 0) {
		arr[0]++
		arr[arr[0]] = carry % 13
		carry = int(carry / 13)
	}
}
# additionne le grand entier src dans dst (les deux en base 13)
function addnum(dst, src, carry, i, t) {
	carry = 0
	i = 1
	while (i <= src[0] || carry > 0) {
		t = dst[i] + carry
		if (i <= src[0])
			t = t + src[i]
		dst[i] = t % 13
		carry = int(t / 13)
		if (i > dst[0])
			dst[0] = i
		i++
	}
}
'
