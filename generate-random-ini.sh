#!/bin/sh -e

random_mac()
{
	dd if=/dev/urandom bs=6 count=10000 2>/dev/null \
	| od -An -x \
	| tr -cd '0-9a-f' \
	| sed -r 's/(..)(..)(..)(..)(..)(..)/00:\2:\3:\4:\5:\6$/g' \
	| tr '$' '\n'
}

random_hosts()
{
	dd if=/dev/urandom bs=1000 count=1 2>/dev/null \
	| od -An -tx1 \
	| sed 's/ //' \
	| xargs -n 3 | sed 's/ //' | xargs -n 3 | sed 's/ //' \
	| xargs -n 4 | sed 's/ //' | xargs -n 5 | sed 's/ //'
}

random_ipv4()
{
	local i=0

	while [ "$i" -lt 1000 ]; do i=$((i + 1))
		echo $((RANDOM % 255 + 1)).$((RANDOM % 255 + 1)).$((RANDOM % 255 + 1)).$((RANDOM % 255 + 1))
	done
}

random_ipv6()
{
	dd if=/dev/urandom bs=6 count=10000 2>/dev/null \
        | od -An -x \
	| sed 's/^ //; s/ /:/g'
}

hosts=$(random_hosts)
ipv4=$(random_ipv4)
ipv6=$(random_ipv6)
mac=$(random_mac)

for h1 in $hosts; do
	echo
	echo "[host]"
	echo "name = $h1"

	for ip in $ipv4; do i=$((i + 1))
		[ $i -lt $((RANDOM % 1000)) ] || break
		echo "ip = $ip"
	done | tail -n $((RANDOM % 100 / (RANDOM % 3 + 1)))

	for ip in $ipv6; do i=$((i + 1))
		[ $i -lt $((RANDOM % 1000)) ] || break
		echo "ip = $ip"
	done | tail -n $((RANDOM % 100 / (RANDOM % 3 + 1)))

	for h2 in $hosts; do i1=$((i1 + 1))
		[ $i1 -lt $((RANDOM % 1000)) ] || break
		echo "link = $h2"
	done | tail -n $((RANDOM % 100 / (RANDOM % 3 + 1)))

	for ip in $ipv4; do i=$((i + 1))
		[ $i -lt $((RANDOM % 1000)) ] || break
		echo "link = $ip"
	done | tail -n $((RANDOM % 100 / (RANDOM % 3 + 1)))

	for ip in $ipv6; do i=$((i + 1))
		[ $i -lt $((RANDOM % 1000)) ] || break
		echo "link = $ip"
	done | tail -n $((RANDOM % 100 / (RANDOM % 3 + 1)))
done
