        org $100
start   lds #$400
        ldb #'a
loop    ldx #data
        lda #5
        clrb
loop2   addb ,x+
        deca
        bne loop2
        cmpb #15
        swi
data    fcb 1,2,3,4,5
        fdb $1234
        fcc "HI"
        end
