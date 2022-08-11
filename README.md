# where-to-eat

Suggests a place for lunch to go based on the places and scores you have given.

No, it doesn't suggests a place randomly, it tries to be fair according to `points` of the `places`.

### How fair is it ?

`wte` treats the lowest scored place is the most desired place. So if you want to go some place more often, then decrease its points from `places` file.

### How to define `places` ?

`wte` reads places from the file which is named `places` within the working directory so manipulate it as you wish.

### `wte` is cumulative

`wte` doesn't forget the suggestions it gave, it writes every suggestions to `.placement` files, so it can be run again and again.

### Schedule next 20 days

Execute the script below in the build directory.

```sh
N_DAYS=20

for (( i = 1; i < ${N_DAYS}; i++ ))
do
    date -d "+${i} day" "+%A - %d %B %Y"
    echo "   " $(./wte)
    echo
done

echo
echo "===================================="
echo "Total collected points for ${N_DAYS} days"
echo "===================================="
./wte summary
```

#### Output
```console
Friday - 12 August 2022
    1 26 sozeri

Saturday - 13 August 2022
    3 24 yesilvadi

Sunday - 14 August 2022
    6 27 disardan

Monday - 15 August 2022
    2 11 bilkent

Tuesday - 16 August 2022
    0 11 duveroglu

Wednesday - 17 August 2022
    5 25 adali

Thursday - 18 August 2022
    4 37 cepa

Friday - 19 August 2022
    7 10 hacettepe

Saturday - 20 August 2022
    7 10 hacettepe

Sunday - 21 August 2022
    2 11 bilkent

Monday - 22 August 2022
    0 11 duveroglu

Tuesday - 23 August 2022
    7 10 hacettepe

Wednesday - 24 August 2022
    2 11 bilkent

Thursday - 25 August 2022
    3 24 yesilvadi

Friday - 26 August 2022
    0 11 duveroglu

Saturday - 27 August 2022
    6 27 disardan

Sunday - 28 August 2022
    5 25 adali

Monday - 29 August 2022
    1 26 sozeri

Tuesday - 30 August 2022
    7 10 hacettepe


====================================
Total collected points for 20 days
====================================
 place total       desc
     0    33  duveroglu
     1    52     sozeri
     2    33    bilkent
     3    48  yesilvadi
     4    37       cepa
     5    50      adali
     6    54   disardan
     7    40  hacettepe
```

### Credits

Thanks to __Ahmet Ilgin__ for the algorithm.
