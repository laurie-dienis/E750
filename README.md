# Analysis of experiment e750
Code of the analysis of the experiment e750, a resonant elastic scattering experiment using ACTAR TPC. The code is using [ACTROOT](https://github.com/loopset/ActRootV2.git) to run.

## To convert the data : 

```
actroot -r tpc
```
```
actroot -r sil
```
```
actroot -f
```
```
actroot -m
```

## To visualize the data event by event: 
```
actplot -v
```
